function processReqChangeLoop()
{
    if (lreq.readyState == 4)
    {
        if (lreq.status == 200 || lreq.status == 304)
        {
            response = lreq.responseXML.documentElement;

            if (requests.getItem(0,1).mutexIndex)
                Mutex.finish(requests.getItem(0,1).mutexIndex);

            if (requests.getItem(0,1).id == "reboot")
            {
                try
                {
                    if (reboottrials > 3)
                    {
                        reboottrials = 0;
                        rebootcount = 0;
                        is_rebooting = false;
                        window.clearInterval(g_interval);
                        new Mutex(show_type,new ShowData(lastmenu),0,"",200);
                    }
                    else reboottrials++;

                    return;
                }
                catch (e) {}

            }
            else
            {
                var text, k, i, tmp;

                try
                {
                    text = response.getElementsByTagName("ppRdData")[0].firstChild.data;
                }
                catch (e)
                {
                    return;
                }

                text = decode_base64(text);

                if ((requests.getItem(0,1).index == browseindex + 4) && requests.getItem(0,1).subindex == 1)
                {
                    if (requests.getItem(0,1).id == 'GetFavVals') parse_plc_vars(text, "f");
                    else parse_plc_vars(text, "");

                    return;
                }

                k = requests.length(1) * 4;

                for (i = 0; i < requests.length(1); i++)
                {
                    tmp = eval_data(k, i, text, requests.getItem(i,1).len, requests.getItem(i,1).type);
                    c_val = tmp.c_val;
                    k = tmp.k;

                    try
                    {
                        document.getElementById("v" + requests.getItem(i,1).subindex + "," + requests.getItem(i,1).index).innerHTML = format(requests.getItem(i,1).i, c_val, -1, -1);
                    }
                    catch (e)
                    {
			try
			{
                        	document.getElementById("a" + requests.getItem(i,1).subindex + "," + requests.getItem(i,1).index).innerHTML = format(requests.getItem(i,1).i, c_val, -1, "a" + requests.getItem(i,1).subindex + "," + requests.getItem(i,1).index);
			}
	                catch (e)
	                {
        	                break;
			}
                    }
                }
            }
        }
        else
        {
            if (requests.getItem(0,1).id == "reboot") reboottext = "Initializing Network";

            if (requests.getItem(0,1).mutexIndex)
                Mutex.finish(requests.getItem(0,1).mutexIndex);
        } 
    }
}

function processReqChange()
{
    if (req.readyState == 4)
    {
        var hasFailed, prefix, c_requests, type, size, val, text, dataName, state, plci, i;
        var idx, sidx, flag, len, type, c_val;
        hasFailed = false;
        prefix = "u:";

        try
        {
            response = req.responseXML.documentElement;
            response.getElementsByTagName(prefix + "errorCode")[0].firstChild.data;
            hasFailed = true;
        }
        catch (e) {}

        if ((req.status == 200 || req.status == 304) && !hasFailed)
        {
            if (requests.length(0) == 0)
            {
                debugln("Empty request list");
                return;
            }
            
            try
            {            
	            var c_requests = requests.get(0);
	
	            debugln("Evaluating <b>" +  c_requests[0].name + "</b> (ID: " + c_requests[0].id + "; Idx: " + c_requests[0].index + "; sIdx: " + c_requests[0].subindex + ")");
	
	            response = req.responseXML.documentElement;
	
	            errors = 0;
	
	            dataName = "ppData";
	
	            if (g_method == "ReadWrite") dataName = "ppRdData";
	
	            try
	                // read / readwrite
	            {
	                response.normalize();
	                text = response.getElementsByTagName(dataName)[0].firstChild.data;
	
	            }
	            catch (e)
	                // write
	            {
	                if (c_requests[0].index == rebootindex) return;
	
	                if (c_requests[0].index == 0x1008)
	                {
	                    return;
	                }
	
	                if (c_requests[0].index > 45056 && c_requests[0].index < 49125)
	                    // b object
	                {
	                    var qData = new QueryData(c_requests[0].index, 2, 0, 1, "int", "", c_requests[0].id, false);
	                    qData.refresh = c_requests[0].refresh;
	                    new Mutex(query_read,qData,0,"");
	                }
	                else if (c_requests[0].refresh)
	                {
	                    new Mutex(show_type,new ShowData(lastmenu),0,"",5000);
	                }
	
	                return;
	            }
	
	            text = decode_base64(text);
	
	            if (c_requests[0].id == "GetFav")
	            {
	                parse_ds(text, "FAVG");
	                Mutex.notify("initDone");
	                return;
	            }
	
	            if (c_requests[0].id == "GetFavList")
	            {
	                parse_ds(text, "FAV");
	                return;
	            }

	            if (c_requests[0].id == 'RunQueryWrite')
                {
	                Mutex.notify("RunQueryWriteFinished");
	            }

	            if (c_requests[0].index == folderindex && c_requests[0].subindex == 1)
	            {
	                parse_dirs(text);
	                load_end();
	                return;
	            }
	
	            if (c_requests[0].index == browseindex && c_requests[0].subindex == 1)
	            {
	                parse_plc(text);
	                return;
	            }
	
	            if (c_requests[0].subindex == 1 && c_requests[0].id == 'DataStoreGet')
	            {
	                parse_ds(text, "DS");
	                return;
	            }
	
	            if (c_requests[0].subindex == 1 && c_requests[0].id == 'CustomerPagesGet')
	            {
	                parse_ds(text, "CP");
	            }
	
	            if ((c_requests[0].index == browseindex + 4) && c_requests[0].subindex == 1)
	            {
	                if (c_requests[0].id == 'GetFavVals') parse_plc_vars(text, "f");
	                else
	                {
	                    parse_plc_vars(text, "");
	                    load_end();
	                }
	
	                return;
	            }
	
	            if (c_requests[0].index == (browseindex + 2) && c_requests[0].subindex == 1)
	            {
	                type = (text.charCodeAt(5) << 24 | text.charCodeAt(4) << 16 | text.charCodeAt(3) << 8 | text.charCodeAt(2));
	                size = (text.charCodeAt(9) << 24 | text.charCodeAt(8) << 16 | text.charCodeAt(7) << 8 | text.charCodeAt(6));
	
	                val = get_plc_val(type, size, text.substring(10));
	                return;
	            }
	
	            if (c_requests[0].index > 45056 && c_requests[0].index < 49125) // b object
	            {
	                if (c_requests[0].subindex == 2)
	                {
	                    state = text.charCodeAt(0);
	                    
	                    if (state != 1 && state != 0)
	                    {
	                        if (state == 3)
	                        {
	                            var qData = new QueryData(c_requests[0].index, 3, 0, 80, "int", "", c_requests[0].id, false);
	                            qData.refresh = c_requests[0].refresh;
	                            new Mutex(query_read,qData,0,"")
	                        }
	                        else
	                        {
	                            errors++;
	                            show_info("Loading... (" + state + ")", "green");
	                            query_sr(new SrData(g_method, false, g_db_sr));
	                        }
	                    }
	                    else // loading complete
	                    {
	                        if (c_requests[0].id == "REMOVEFAVGROUP")
	                        {
	                            plci = get_object("PLC");
	                            mod_list[plci].favorites.deleteByElem(currenttemp);
	                            for (i = document.getElementById("favgroups").length - 1; i >= 0; i--)
	                            {
	                                if (document.getElementById("favgroups").options[i].text == currenttemp)
	                                {
	                                    document.getElementById("favgroups").remove(i);
	                                }
	                            }
	
	                            c_requests[0].refresh = true;
	                        }
	                        else if (c_requests[0].id == "ADDFAVGROUP")
	                        {
	                            var ngroup = true;
	                            plci = get_object("PLC");
	
	                            if (plci == -1)
	                            {
	                                return;
	                            }
	
	                            for (i = 0; i < mod_list[plci].favorites.length; i++)
	                            {
	                                if (currenttemp == mod_list[plci].favorites[i]) ngroup = false;
	                            }
	
	                            if (ngroup)
	                            {
	                                mod_list[plci].favorites.push(currenttemp);
	                                document.getElementById("favgroups").options[document.getElementById("favgroups").length] = new Option(currenttemp, currenttemp);
	                            }
	
	                            currenttemp = [];
	                        }
	
	                        if (c_requests[0].refresh)
	                        {
	                            new Mutex(show_type,new ShowData(lastmenu),0,"",200);
	                        }
	                    }
	                }
	                else if (c_requests[0].subindex == 3)
	                {
	                    var errCode = (text.charCodeAt(5) << 24 | text.charCodeAt(4) << 16 | text.charCodeAt(3) << 8 | text.charCodeAt(2));
	                    clear_info();
	
	                    if (errCode == -324597584) show_info("Error: The user account already exists!", "red");
	                    else show_info("Error (" + errCode + "): Command failed!", "red");
	                }
	
	                return;
	            }

	            var a_err = [];
							
	            if (c_requests[0].name == "WriteMultiple")
	            {
									
	                for (i = 0; i < c_requests.length; i++)
	                {
	                    var j = i * 4;
	                    a_err[i] = (text.charCodeAt(j + 3) << 24 | text.charCodeAt(j + 2) << 16 | text.charCodeAt(j + 1) << 8 | text.charCodeAt(j));
	
	                    if (a_err[i] != 0) show_info("Error (" + a_err[i] + "): Unable to change " + mod.get_request_variable(i).name, "red");
	
                        try	
                        {				
                            // Edit by Sebastian Glatz on 04.11.2009 
                            if(document.getElementById("v" + mod.get_request_variable(i).subindex + "," + mod.get_request_variable(i).index + "info") != null)
                            document.getElementById("v" + mod.get_request_variable(i).subindex + "," + mod.get_request_variable(i).index + "info").style.display = "none";	
															
                            if(document.getElementById("a" + mod.get_request_variable(i).subindex + "," + mod.get_request_variable(i).index + "info") != null)
                            document.getElementById("a" + mod.get_request_variable(i).subindex + "," + mod.get_request_variable(i).index + "info").style.display = "none";	
                        }
                        catch (e) 
                        {
                            break;
                        }

                    }
									
	
	                mod.show(false);
	
	                apply_button_enable(false);
	
	                if (mod.needreboot) rebootWithQuestion("Settings saved.");
	
	                return;
	            }
	
	            var arrlenset = false;
	
	            if (c_requests[0].name == "ReadMultiple" && c_requests[0].index != 0xf020)
	            {
	                a_err = [];
	                var k = c_requests.length * 4;
	                var lasttype = "";
	                var doblock = false;
	
	                for (i = 0; i < c_requests.length; i++)
	                {
	                    var tmp = eval_data(k, i, text, c_requests[i].len, c_requests[i].type);
	                    c_val = tmp.c_val;
	                    k = tmp.k;
	                    a_err[i] = tmp.error;
	
	                    if (c_requests[i].index % 16 == 0)
	                        // type object
	                    {
	
	                        if (c_requests[i].subindex == 3)
	                            // name
	                        {
	                            mod_list[mod_count++] = new Module(c_val, lasttype, c_requests[i].index, c_requests[i].type);
	
	                            debugln(c_val + "," + lasttype + ";");
	
	                            if (lasttype == "DataStore" && c_val == "System") systemdsindex = c_requests[i].index + 12288;
	
	                            if (c_requests.length == (i + 1)) // last item
	                            {
	                                if (c_requests[0].mutexIndex)
	                                    Mutex.finish(c_requests[0].mutexIndex);
	
	                                if (systemdsindex != -1)
	                                {
	                                    var folder = "CP";
	                                    var ndata = [];
	                                    ndata = prepare_data(16 + folder.length + 1, "int", 4, ndata);
	                                    // cbData
	                                    ndata = prepare_data(0, "int", 4, ndata);
	                                    // hContinue
	                                    ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
	                                    // maxReadLen
	                                    ndata = prepare_data(folder.length + 1, "int", 4, ndata);
	                                    // cbPath
	                                    ndata = prepare_data(folder, "string", 0, ndata);
	                                    full_buffer = [];
	                                    full_buffer_param = folder;
	
	                                    new Mutex (query_readwrite, new QueryData(systemdsindex, 1, 2, buffer_size, "data", ndata, "CustomerPagesGet", false), 0);
	                                }
	                                else
	                                {
	                                    Mutex.notify("initDone");
	                                }
	
	                            }
	
	                        }
	                        else
	                        {
	                            lasttype = c_val;
	                            if (c_val == "Misc") rebootindex = c_requests[i].index + 12289;
	                            if (c_val == "FSO") folderindex = c_requests[i].index + 12288;
	                            if (c_val == "PLC") browseindex = c_requests[i].index + 12288;
	                        }
	
	                    }
	                    else // everything else
	                    {
	                        if (c_requests[i].id == "init")
	                        {
	                            if (c_requests[i].index == 0x1008)
	                            {
	                                if (c_val == "-2147220020") c_val = "Unknown";
	
	                                document.getElementById("mhostname").innerHTML = c_val;
	
	                                general_info.hostname = c_val;
	                            }
	                            else if (c_requests[i].index == 0x1009)
	                            {										
																		if (c_val != "??????;v??.?" && c_val != "??0000;v00.0;0000-00-00")
																		{
																			var giHwStr = "";
																			var giHwArr = c_val.split(";");
																			if(giHwArr[0] && (giHwArr[0]!="??0000" && giHwArr[0]!="000000" && giHwArr[0]!="??????" )){
																				var giHwName = giHwArr[0];
																				giHwName = giHwName.replace(/\?/g,"");
																				giHwStr += ""+giHwName;
																			}
																			if(giHwArr[1] && (giHwArr[1]!="v00.0" && giHwArr[1]!="v??.?")){
																				var giHwVersion = giHwArr[1];
																				giHwStr += ";"+giHwVersion;
																			}
																			if(giHwArr[2] && giHwArr[2]!="0000-00-00"){
																				var giHwDateArr = giHwArr[2].split("-");
																				var giHwDateYear = giHwDateArr[0];
																				var giHwDateMonth = giHwDateArr[1];
																				var giHwDateDay = giHwDateArr[2];
																				giHwDateYear = parseFloat(giHwDateYear) + 2000;
																				var giHwDateStr = giHwDateYear+"-"+giHwDateMonth+"-"+giHwDateDay;
																				giHwStr += ";"+giHwDateStr;
																			}
																			if(giHwStr!="")
																				general_info.hardware = giHwStr;
																			else 
																				general_info.hardware = null;		
																		}
																		else 
																			general_info.hardware = null;	
	                            }
	                            else if (c_requests[i].index == 0x100A)
	                            {
	                            	  if (c_val != "???")
	                                	general_info.os = c_val;
	                                else general_info.os = null;
	                            }
	                        }
	                        else if (c_requests[i].id == "Timezone")
	                        {
	                            if (c_requests[i].subindex != 0)
	                            {
	                                try
	                                {
	                                    var tzcurrent = document.getElementById("v5," + (mod.index + 1)).firstChild.innerHTML;
	                                    currenttemp.push(c_val);
	
	                                    if (currenttemp.length - 1 == tzcurrent)
	                                    {
	                                        document.getElementById("v5," + (mod.index + 1)).firstChild.innerHTML = c_val;
	                                        document.getElementById("v5," + (mod.index + 1) + "old").value = c_val;
	                                    }
	                                }
	                                catch (e) {}
	
	                            }
	
	                        }
	                        else if (c_requests[i].id == "Arraylens")
	                        {
	                            if (mod)
	                            {
	                                debugln("ArrayLen set to <b>" + c_val + "</b>");
	                                mod.arrlens.push(c_val);
	                                arrlenset = true;
	                            }
	                        }
	                        else if (c_requests[i].id == "Resolution")
	                        {
	                        	 
	                            if (c_requests[i].subindex != 0)
	                            {                         	  
	                                try
	                                {
	                                    var rescurrent = document.getElementById("v1," + (mod.index + 1)).firstChild.innerHTML;
	                                    currenttemp.push(c_val);
	
	                                    if (currenttemp.length == rescurrent)
	                                    {
	                                        document.getElementById("v1," + (mod.index + 1)).firstChild.innerHTML = c_val;
	                                        document.getElementById("v1," + (mod.index + 1) + "old").value = c_val;
	                                    }
	                                }
	                                catch (e)
	                                {
	                                    debugln(e.description)
	                                }
	                            }
	                        }
	                        else if (mod)
	                        {
	                            var t = i;
	                            prefix = "a";
	                            var carr = 0;
	
	                            if (mod.arrayindexes.length > 0 && mod.arrayindexes[0] < c_requests.length && i >= mod.arrayindexes[0])
	                            {
	                                t = c_requests[i].i;
	
	                                for (var z = mod.arrayindexes.length - 1; z >= 0; z--)
	                                {
	                                    if (i <= mod.arrayindexes[z])
	                                    {
	                                        carr = mod.arrayindexes[z];
	                                        break;
	                                    }
	                                }
	                            }
	                            else prefix = "v";
	
	                            if (a_err[i] || (mod.variables[t].displaytype=="RaidReason" && c_val==0))
	                                // hide empty rows
	                            {
	
	                                try
	                                {
	                                    document.getElementById("tr" + c_requests[i].subindex + "," + c_requests[i].index).style.display = "none";
	                                }
	                                catch (Exception) {}
	
	                            }
	
	                            if (mod.variables[t].editable)
	                            {
	                                if (mod.arraystart == 0) document.getElementById(prefix + c_requests[i].subindex + "," + c_requests[i].index + "old").value = format(t, c_val, -1, -1);

																	document.getElementById(prefix + c_requests[i].subindex + "," + c_requests[i].index).innerHTML = format(t, c_val, t, prefix + c_requests[i].subindex + "," + c_requests[i].index);
	                            }
	                            else if (mod.variables[t].displaytype != "hidden")
	                            {
	                                if (!a_err[i])
	                                {
	                                    document.getElementById(prefix + c_requests[i].subindex + "," + mod.variables[t].index).innerHTML = format(t, c_val, -1, prefix + c_requests[i].subindex + "," + mod.variables[t].index);
	                                }
	
	                                if (mod.favorites && (c_requests[i].index == (mod.index + 1)))
	                                {
	                                    var exists = false;
	
	                                    for (var n = 0; n < document.getElementById("favgroups").length; n++)
	                                    {
	                                        if (c_val == document.getElementById("favgroups").options[n].text) exists = true;
	                                    }
	
	                                    if (!exists)
	                                    {
	                                        document.getElementById("favgroups").options[document.getElementById("favgroups").length] = new Option(c_val, c_val, false, false);
	                                    }
	                                }
	                            }
	                            else
	                            {
	                                document.getElementById(prefix + c_requests[i].subindex + "," + mod.variables[t].index).value = c_val;
	                            }
	
	                            doblock = check_var_disable(t, is_true(c_val));
	                        }
	                        else document.getElementById("current").innerHTML += c_requests[i].index + ": " + c_val + "<br>";
	                    }
	                }
	
	                if (doblock)
	                {
	                    do_var_disable();
	                }
	
	            }
	            else if (c_requests[0].index == 0xf020)
	            {
	                read_modes(text);
	            }
	            else
	            {
	                if (c_requests[0].type == "int")
	                {
	                    c_val = (text.charCodeAt(3) << 24 | text.charCodeAt(2) << 16 | text.charCodeAt(1) << 8 | text.charCodeAt(0));
	                }
	                else if (c_requests[0].type == "bool")
	                {
	                    c_val = text.charCodeAt(0);
	                }
	                else
	                {
	                    c_val = text;
	                }
	            }
	
	            if (arrlenset)
	            {
	                mod.show(true);
	            }
	
	            if (mod && mod.type == "Time" && c_requests[0].id != "Timezone")
	                // add TimeZones
	            {
	                var tzcount = document.getElementById("v0," + (mod.index + 2)).value;
	
	                idx = [];
	                sidx = [];
	                flag = [];
	                len = [];
	                type = [];
	
	                currenttemp = [];
	
	                for (i = 0; i < tzcount; i++)
	                {
	                    idx[i] = mod.index + 2;
	                    sidx[i] = i;
	                    flag[i] = 0;
	                    len[i] = 64;
	                    type[i] = "string";
	                }
	
	                currenttemp = [];
	                new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, len, type, -1, "Timezone", false),0);
	            }
	
	            if (mod && mod.type == "Display Device" && c_requests[0].id != "Resolution") // add Resolutions
	            {
	                debug("Creating resolution query ");
	                var rescount = document.getElementById("v0," + (mod.index + 2)).value;
	                debugln("with " + rescount + " resolutions");
	
	                idx = [];
	                sidx = [];
	                flag = [];
	                len = [];
	                type = [];
	
	                currenttemp = [];
	
	                for (i = 0; i < (parseInt(rescount)+1); i++)
	                {
	                    idx[i] = mod.index + 2;
	                    sidx[i] = i;
	                    flag[i] = 0;
	                    len[i] = 50;
	                    type[i] = "string";
	                }
	
	                new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, len, type, -1, "Resolution", false),0);
	            }
            }
	        catch (e) 
            {
            }
            finally
            {          
	            debugln("uid: "+c_requests[0].mutexIndex);
                
                if (c_requests[0].id!="F020") //unlock the F020 query after loading all objects
	                if (c_requests[0].mutexIndex) //unlock all other queries here
	                    Mutex.finish(c_requests[0].mutexIndex);
            }

	    }
        else
        {
            debugln("Error: Query failed with status <b>" + req.status + "</b>");
            errors++;

				if (requests.length(0) != 0)
            	    if (requests.get(0)[0].mutexIndex)
               	        Mutex.finish(requests.get(0)[0].mutexIndex);

            try
            {
                response = req.responseXML.documentElement;
                var errorCode = response.getElementsByTagName(prefix + "errorCode")[0].firstChild.data;
                var errorDescription = response.getElementsByTagName(prefix + "errorDescription")[0].firstChild.data;
                show_info("Error: Query failed (" + errorCode + ")\r\n" + errorDescription, "red");

                if (errorCode == "710" && errors < maxerrors) new Mutex(query_sr, new SrData(g_method, false, g_db_sr),3);
                else load_end();
            }
            catch (e)
            {
                show_info("Error: Query failed (" + req.status + ")\r\nTrying again", "red");

                if (errors < maxerrors) new Mutex(query_sr, new SrData(g_method, false, g_db_sr),3);
                else load_end();

                return;
            }
        } 
    }
}