function folder_leave(i, isvar)
{
    var id = "selectedfolderid";

    if (isvar)
        id = "selectedvarid";

    edit_leave(i, document.getElementById("selectedfolder").value, document
               .getElementById(id).value);
}

function getIcon(variable)
{
    var icon = "";
    switch (variable.displaytype)
    {

    case "date":
        icon = "Calendar";
        break;

    case "time":
        icon = "Time";
        break;

    case "dir":

    case "plcvar":
        icon = "OZ";
        break;

    default:

        if (variable.editable)
            icon = "edit";
        else
            icon = "";
    }

    return (icon?"<img alt=\"Edit\" title=\"Click on the value to edit\" src=\"images/"
            + icon + ".gif\" border=\"0\">":"");
}

function do_var_disable()
{
    for ( var k = 0; k < mod.variables.length; k++)
    {
        if (!mod.variables[k].editable)
            continue;

        if (mod.variables[k].disable)
        {
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1) + "icon").innerHTML = "";
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1) + "td").parentNode.parentNode.parentNode.parentNode.className = "lockedtable";
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1) + "td").className = "lockedtable";
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1)).innerHTML = "<span style=\"width:90%; display: block;\">"
                                                                   + document.getElementById("v" + mod.variables[k].subindex
                                                                                             + "," + (mod.index + 1)).childNodes[0].innerHTML
                                                                   + "</span>";

        }
        else
        {
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1) + "icon").innerHTML = getIcon(mod.variables[k]);
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1) + "td").parentNode.parentNode.parentNode.parentNode.className = "normaltable";
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1) + "td").className = "normaltable";
            document.getElementById("v" + mod.variables[k].subindex + ","
                                    + (mod.index + 1)).innerHTML = "<span style=\"width:90%; display: block;\" onclick=\"edit('"
                                                                   + k
                                                                   + "',this.innerHTML,'"
                                                                   + "v"
                                                                   + mod.variables[k].subindex
                                                                   + ","
                                                                   + (mod.index + 1)
                                                                   + "')\">"
                                                                   + document.getElementById("v" + mod.variables[k].subindex
                                                                                             + "," + (mod.index + 1)).childNodes[0].innerHTML
                                                                   + "</span>";
        }
    }
}

function check_var_changed(id)
{
    for ( var k = 0; k < mod.variables.length; k++)
    {
        if ((document.getElementById("v" + mod.variables[k].subindex + ","
                                     + (mod.index + 1) + "info") != (id + "info")))
            if (document.getElementById("v" + mod.variables[k].subindex + ","
                                        + (mod.index + 1) + "info") != null)
                if (document.getElementById("v" + mod.variables[k].subindex
                                            + "," + (mod.index + 1) + "info").style.display == "")
                    return true;
    }

    return false;
}

function check_var_disable(j, val)
{
    if (mod.variables[j].need == -2)
    {
        for ( var k = 0; k < mod.variables.length; k++)
        {
            if (j == k || !mod.variables[k].editable)
                continue;

            if (mod.variables[k].need == mod.variables[j].subindex && !val)
            {

                mod.variables[k].disable = false;
            }
            else
                mod.variables[k].disable = true;
        }

        return true;
    }

    return false;
}

function collapse(id)
{
    if (document.getElementById("mdiv_" + id).style.display == "none")
    {
        document.getElementById("micon_" + id).src = "images/MU.gif";
        document.getElementById("mdiv_" + id).style.display = "";
    }
    else
    {
        document.getElementById("micon_" + id).src = "images/PM.gif";
        document.getElementById("mdiv_" + id).style.display = "none";
    }
}

function show_extended(ca, j)
{
    try
    {
        for ( var i = 0;; i++)
        {
            document.getElementById("extendedtable" + ca + "_" + i).style.display = "none";
        }
    }
    catch (e)
    {
    }
    ;

    document.getElementById("extendedtable" + ca + "_" + j).style.display = "";
}

// Display and edit options for values in the add menu
function format_add(type, id, disable)
{
    var browseType = "";

    switch (type)
    {

    case "ed":
        text = "<select class=\"input\" id=\"" + id
               + "\"><option value=\"1\">Enabled</option>";
        text += "<option value=\"0\">Disabled</option></select>";
        break;

    case "routeflags":
        text = "<input type=\"hidden\" id=\"" + id
               + "\" class=\"input\" type=\"text\" style=\"width:98%;\">";
        text += "<input type=\"checkbox\" id=\"c_temp\" onChange=\"javascript:routeflags('"
                + id + "')\"> Temporary <br>";
        text += "<input type=\"checkbox\" id=\"c_dyn\" onChange=\"javascript:routeflags('"
                + id + "')\"> Dynamic <br>";
        break;

    case "transport":
        text = "<select class=\"input\" id=\"" + id + "\">";

        for ( var i = 0; i < transportarr.length; i++)
        {
            text += "<option value=\"" + (i + 1) + "\">" + transportarr[i]
                    + "</option>";
        }

        text += "</select>";
        break;

    case "fwflags":
        text = "<input type=\"hidden\" id=\"" + id
               + "\" class=\"input\" type=\"text\" style=\"width:98%;\">";
        text += "<input type=\"checkbox\" id=\"c_blo\" onChange=\"javascript:fwflags('"
                + id + "')\"> Block <br>";
        text += "<input type=\"checkbox\" id=\"c_all\" onChange=\"javascript:fwflags('"
                + id + "')\"> Allow <br>";
        text += "<input type=\"checkbox\" id=\"c_log\" onChange=\"javascript:fwflags('"
                + id + "')\"> Log <br>";
        text += "<input type=\"checkbox\" id=\"c_inb\" onChange=\"javascript:fwflags('"
                + id + "')\"> Inbound <br>";
        text += "<input type=\"checkbox\" id=\"c_out\" onChange=\"javascript:fwflags('"
                + id + "')\"> Outbound <br>";
        text += "<input type=\"checkbox\" id=\"c_dis\" onChange=\"javascript:fwflags('"
                + id + "')\"> Disabled <br>";
        break;

    case "fwaction":
        text = "<select class=\"input\" id=\"" + id
               + "\" DISABLED><option value=\"1\">Block</option>";
        text += "<option value=\"2\">Allow</option><option value=\"3\">Allow Response</option></select>";
        break;

    case "protocol":
        text = "<select class=\"input\" id=\"" + id
               + "\" DISABLED><option value=\"6\">TCP</option>";
        text += "<option value=\"17\">UDP</option></select>";
        break;

    case "portrange":
        text = "<input type=\"hidden\" id=\"" + id + "\">";
        text += "Min: <input id=\"pmin\" class=\"input\" type=\"text\" style=\"width:80%;\" DISABLED onChange=\"javascript:hl('"
                + id + "','pmin','pmax','4')\"><br>";
        text += "Max: <input id=\"pmax\" class=\"input\" type=\"text\" style=\"width:80%;\" DISABLED onChange=\"javascript:hl('"
                + id + "','pmin','pmax','4')\">";
        break;

    case "typecode":
        text = "<input type=\"hidden\" id=\"" + id + "\">";
        text += "Type: <input id=\"ptype\" class=\"input\" type=\"text\" style=\"width:80%;\" DISABLED onChange=\"javascript:hl('"
                + id + "','ptype','pcode','2')\"><br>";
        text += "Code: <input id=\"pcode\" class=\"input\" type=\"text\" style=\"width:80%;\" DISABLED onChange=\"javascript:hl('"
                + id + "','ptype','pcode','2')\">";
        break;

    case "mask":
        text = "<input type=\"hidden\" id=\"" + id + "\">";
        text += "<input type=\"checkbox\" id=\"c_pri\" onChange=\"javascript:fwmask('"
                + id + "')\"> Private Host <br>";
        text += "<input type=\"checkbox\" id=\"c_pub\" onChange=\"javascript:fwmask('"
                + id + "')\"> Public Host <br>";
        text += "<input type=\"checkbox\" id=\"c_por\" onChange=\"javascript:fwmask('"
                + id + "')\"> Port <br>";
        text += "<input type=\"checkbox\" id=\"c_typ\" onChange=\"javascript:fwmask('"
                + id + "')\"> Type <br>";
        text += "<input type=\"checkbox\" id=\"c_cod\" onChange=\"javascript:fwmask('"
                + id + "')\"> Code <br>";
        text += "<input type=\"checkbox\" id=\"c_pro\" onChange=\"javascript:fwmask('"
                + id + "')\"> Protocol <br>";
        text += "<input type=\"checkbox\" id=\"c_act\" onChange=\"javascript:fwmask('"
                + id + "')\"> Action <br>";
        text += "<input type=\"checkbox\" id=\"c_mas\" onChange=\"javascript:fwmask('"
                + id + "')\"> Public Host Mask <br>";
        text += "<input type=\"checkbox\" id=\"c_pre\" onChange=\"javascript:fwmask('"
                + id + "')\"> Public Host Prefix <br>";
        break;

    case "dir":
        var value = "\\";

        if (winxp && value.charAt(1) != ":")
            value = "C:" + value;

        text = "<input type=\"hidden\" id=\"" + id + "\">";

        text += "<span id=\"currentfoldercontainer\"><input onkeypress=\"folderenter(event);\" id=\"selectedfolder\" class=\"input\" type=\"text\" style=\"width:98%;\" value=\""
                + value
                + "\"><input id=\"selectedfolderid\" type=\"hidden\" value=\""
                + id
                + "\"><br><div id=\"subfolders\">loading subfolders...</div></span>";

        window.setTimeout("folderchange('" + double_slashes(value)
                          + "', '', 'DIR', '')", 4000);

        break;

    case "dsvar":
        id = "DataStoreGet";

        browseType = "DS";

    case "plcvar":
        text = "<input type=\"hidden\" id=\"" + id + "\">";

        text += "<span id=\"currentfoldercontainer\"><input id=\"selectedvar\" class=\"input\" type=\"text\" style=\"width:98%;\" value=\"\"><input id=\"selectedvarid\" type=\"hidden\" value=\""
                + id + "\"><br><div id=\"varfolders\">loading...</div></span>";

        if (browseType != "DS")
            browseType = "PLC";

        window.setTimeout("folderchange('', '', '" + browseType + "', '" + id
                          + "')", 400);

        break;

    default:
        text = "<input id=\"" + id
               + "\" class=\"input\" type=\"text\" style=\"width:98%;\" "
               + (disable ? "DISABLED" : "") + ">";

        break;
    }

    return text;

}

function folderenter(e)
{
    if (!e)
        e = window.event;

    var keyCode = e.keyCode || e.which;

    if (keyCode == 13)
    {
        var folder = document.getElementById("selectedfolder").value;

        if (folder.length <= 0)
            return;

        if (folder.charAt(folder.length - 1) != "\\")
            folder += "\\";

        var ndata = [];

        ndata = prepare_data(folder.length + 1, "int", 4, ndata);

        ndata = prepare_data(folder + "*", "string", 0, ndata);

        new Mutex (query_readwrite, new QueryData(folderindex, 1, 2, buffer_size, "data", ndata, "", false), 0);

        folderparent = folder;

        document.getElementById(document.getElementById("selectedfolderid").value).value = folder;

        cancelreturn = true;
    }
}

function hl(id, v1, v2, size)
{
    var val1 = document.getElementById(v1).value;
    var val2 = document.getElementById(v2).value;
    document.getElementById(id).value = (val1 | (val2 << (size * 4)));
}

function fwmask(id)
{
    try
    {
        var pri = document.getElementById("c_pri").checked ? 4096 : 0;
        var pub = document.getElementById("c_pub").checked ? 2048 : 0;
        var por = document.getElementById("c_por").checked ? 64 : 0;
        var typ = document.getElementById("c_typ").checked ? 32 : 0;
        var cod = document.getElementById("c_cod").checked ? 16 : 0;
        var pro = document.getElementById("c_pro").checked ? 8 : 0;
        var act = document.getElementById("c_act").checked ? 4 : 0;
        var mas = document.getElementById("c_mas").checked ? 2 : 0;
        var pre = document.getElementById("c_pre").checked ? 1 : 0;

        if (pri)
            document.getElementById("aPrivate Host").disabled = false;
        else
            document.getElementById("aPrivate Host").disabled = true;

        if (pub)
            document.getElementById("aPublic Host").disabled = false;
        else
            document.getElementById("aPublic Host").disabled = true;

        if (por)
            document.getElementById("pmin").disabled = false;
        else
            document.getElementById("pmin").disabled = true;

        if (por)
            document.getElementById("pmax").disabled = false;
        else
            document.getElementById("pmax").disabled = true;

        if (pro)
            document.getElementById("aProtocol").disabled = false;
        else
            document.getElementById("aProtocol").disabled = true;

        if (act)
            document.getElementById("aActions").disabled = false;
        else
            document.getElementById("aActions").disabled = true;

        if (mas)
            document.getElementById("aPublic Host Mask").disabled = false;
        else
            document.getElementById("aPublic Host Mask").disabled = true;

        if (typ)
            document.getElementById("ptype").disabled = false;
        else
            document.getElementById("ptype").disabled = true;

        if (cod)
            document.getElementById("pcode").disabled = false;
        else
            document.getElementById("pcode").disabled = true;

        document.getElementById(id).value = pre | mas | act | pro | cod | typ
                                            | por | pub | pub;
    }
    catch (e)
    {
    }

}

function fwflags(id)
{
    try
    {
        var dis = document.getElementById("c_dis").checked ? 32 : 0;
        var out = document.getElementById("c_out").checked ? 16 : 0;
        var inb = document.getElementById("c_inb").checked ? 8 : 0;
        var log = document.getElementById("c_log").checked ? 4 : 0;
        var all = document.getElementById("c_all").checked ? 2 : 0;
        var blo = document.getElementById("c_blo").checked ? 1 : 0;

        if (blo)
            document.getElementById("c_all").disabled = true;
        else
            document.getElementById("c_all").disabled = false;

        if (all)
            document.getElementById("c_blo").disabled = true;
        else
            document.getElementById("c_blo").disabled = false;

        document.getElementById(id).value = blo | all | log | inb | out | dis;
    }
    catch (Exception)
    {
    }

}

/*
 * function userflags(id) { try { var
 * smb=document.getElementById("c_smb").checked?4:0; var
 * ftp=document.getElementById("c_ftp").checked?2:0; var
 * ras=document.getElementById("c_ras").checked?1:0;
 *
 * document.getElementById(id).value=ras|smb|ftp; } catch (Exception){} }
 */

function routeflags(id)
{
    var temp = document.getElementById("c_temp");
    var dyn = document.getElementById("c_dyn");

    var retval = 4;

    if (document.getElementById("c_temp").checked
            && document.getElementById("c_dyn").checked)
        retval = 7;
    else if (document.getElementById("c_temp").checked)
        retval = 5;
    else if (document.getElementById("c_dyn").checked)
        retval = 6;

    document.getElementById(id).value = retval;
}

function change_line(index, subindex, value)
{
    if (value == 1)
        value = 0;
    else
        value = 1;

    var qData = new QueryData(index, subindex, 0, 0, "bool", value, "", false);
    new Mutex(query_write,qData,0);
}

// Display and edit options for received values
function format(j, value, i, id)
{
    value = value.toString();

    switch (mod.variables[j].displaytype)
    {

    case "hl":

        if (mod.variables[j].len == 2)
            value = (value & 0xff) + " - " + ((value >> 8) & 0xff);
        else
            // len==4
            value = (value & 0xffff) + " - " + ((value >> 16) & 0xffff);

        break;

    case "fwaction":
        if (value == 0)
            value = "None";
        else if (value == 1)
            value = "Block";
        else if (value == 2)
            value = "Allow";
        else if (value == 3)
            value = "Allow Response";

        break;

    case "protocol":
        if (value == 6)
            value = "TCP";
        else if (value == 17)
            value = "UDP";

        break;
        
    case "RaidDrives":
    	  var oldvalue = value;
    	  value = "";
        for (var k = 0; k < oldvalue.length; k++)
        {
        	var driveidx=parseInt(oldvalue.charCodeAt(k));
			

            	if (!id.startsWith("a")) {value = "ERROR"; break;}
            	//get idx of "drives"
            	var idx = parseInt(id.substring(id.length-5)); 

            	//add SN and state
		if (k!=0) value+="<br>";
            	value += document.getElementById("a"+driveidx+","+(idx+5)).innerHTML + " - "+document.getElementById("a"+driveidx+","+(idx+6)).innerHTML;

        }
        break;
    case "DriveState":
    	 switch (parseInt(value))
    	 {
    	 	case 0: value = "OK"; break;
    	 	case 1: value = "REBUILDING"; break;
    	 	case 2: value = "FAILED"; break;
    	 	case 3: value = "DEGRADED"; break;
		 }
		 break;
    case "RaidState":
    	 switch (parseInt(value))
    	 {
    	 	case 1: value = "GOOD"; break;
    	 	case 2: value = "FAILED"; break;
    	 	case 3: value = "OFFLINE"; break;
    	 	case 4: value = "POWEROFF"; break;
		 }
		 break;
    case "RaidReason":
    	 switch (parseInt(value))
    	 {
    	 	case 0: value = "NO_REASON"; break;
    	 	case 1: value = "INITIALIZING"; break;
    	 	case 2: value = "BUS_DEGRADED"; break;
    	 	case 3: value = "BUS_FAILURE"; break;
		 }
		 break;
    case "RaidType":
    	 switch (parseInt(value))
    	 {
    	 	case 0: value = "No Raid"; break;
    	 	case 1: value = "Raid Type 0"; break;
    	 	case 2: value = "Raid Type 1"; break;
    	 	case 3: value = "Raid Type 10"; break;
    	 	case 4: value = "Raid Type 5"; break;
    	 	case 5: value = "Raid Type 15"; break;
    	 	case 255: value = "No Standard Type"; break;
		 }
		 break;
    case "RaidInfo":
    	 value = parseInt(value);
	 if (value==0) value = "OK";
		 else if ((value&0x00FF)==1) value = "Degraded";
		 else if ((value&0x00FF)==2) value = "Rebuilding";
		 else value = "Failed";
		 break;
    case "fwflags":
        oldval = value;

        value = "";

        if ((oldval & 1) == 1)
            value += (value == "" ? "" : ", ") + "Block";

        if ((oldval & 2) == 2)
            value += (value == "" ? "" : ", ") + "Allow";

        if ((oldval & 4) == 4)
            value += (value == "" ? "" : ", ") + "Log";

        if ((oldval & 8) == 8)
            value += (value == "" ? "" : ", ") + "Inbound";

        if ((oldval & 16) == 16)
            value += (value == "" ? "" : ", ") + "Outbound";

        if ((oldval & 32) == 32)
            value += (value == "" ? "" : ", ") + "Disabled";

        break;
        
    case "mb":
    		value = Math.round(value / 1024 / 1024);
    		value += " MB";
    		break;

    case "mask":
        oldval = value;

        value = "";

        if ((oldval & 1) == 1)
            value += (value == "" ? "" : ", ") + "Private Host";

        if ((oldval & 2) == 2)
            value += (value == "" ? "" : ", ") + "Public Host";

        if ((oldval & 4) == 4)
            value += (value == "" ? "" : ", ") + "Port";

        if ((oldval & 8) == 8)
            value += (value == "" ? "" : ", ") + "Type";

        if ((oldval & 16) == 16)
            value += (value == "" ? "" : ", ") + "Code";

        if ((oldval & 32) == 32)
            value += (value == "" ? "" : ", ") + "Protocol";

        if ((oldval & 64) == 64)
            value += (value == "" ? "" : ", ") + "Action";

        if ((oldval & 2048) == 2048)
            value += (value == "" ? "" : ", ") + "Public Host Mask";

        if ((oldval & 4096) == 4096)
            value += (value == "" ? "" : ", ") + "Private Host Mask";

        break;

    case "yn":
        if (is_true(value))
            value = "Yes";
        else
            value = "No";

        break;

    case "bool":

    case "tf":
        if (is_true(value))
            value = "True";
        else
            value = "False";

        break;

    case "ed":
        if (is_true(value))
            value = "Enabled";
        else
            value = "Disabled";

        break;

    case "x":
        if (is_true(value))
            value = "<div align=\"center\" width=\"100%\"><img src=\"images\\check.gif\"></div>";
        else
            value = "";

        break;

    case "security":
        if (value == 0)
            value = "Disabled (0)";
        else if (value == 1)
            value = "Webpage (1)";
        else if (value == 2)
            value = "Device (2)";

        break;

    
		case "ewf-state":
		
			switch (value)
			{
				case "0":
            value = "Enabled";
            break;

        case "1":
            value = "Disabled";
            break;
			}
			
		break;
		
		case "ewf-type":
		
			switch (value)
			{
				case "0":
            value = "Disk";
            break;

        case "1":
            value = "RAM";
            break;
						
				case "2":
            value = "RAM_REG";
            break;
			}
			
		break;
		
		case "ewf-bootcommand":
        switch (value)
        {

        case "0":
            value = "NoCommand";
            break;

        case "1":
            value = "Enable";
            break;

        case "2":
            value = "Disable";
            break;

        case "3":
            value = "SetLevel";
            break;

        case "4":
            value = "Commit";
            break;

        default:
            value = "";
        }

        break;

    case "rf":
        var tmp = "";
        tmp += (value & 1 ? "<b>" : "") + "Temporary<br>"
               + (value & 1 ? "</b>" : "");
        tmp += (value & 2 ? "<b>" : "") + "Dynamic<br>"
               + (value & 2 ? "</b>" : "");
        tmp += (value & 4 ? "<b>" : "") + "No Override<br>"
               + (value & 4 ? "</b>" : "");
        value = tmp;
        break;

    case "transport":
        value = transportarr[value];
        break;

    case "tcreglvl":

        switch (value)
        {

        case "0":
            value = "CP";
            break;

        case "1":
            value = "I/O";
            break;

        case "2":
            value = "PLC";
            break;

        case "3":
            value = "NC-PTP";
            break;

        case "4":
            value = "NC-I";
            break;
        }

        break;

    case "tcstate":

        switch (value)
        {

        case "5":
            value = "RUN";
            break;

        case "6":
            value = "STOP";
            break;

        case "15":
            value = "CONFIG";
            break;

        default:
            value = "ERROR";
        }

        value = "<table><tr><td class=\"" + value + "\">" + value
                + "</td></tr></table>";
        break;

    case "time":
        var date = new Date(parseFloat(value) * 1000);
        var dd = date.getUTCDate();
        var mm = date.getUTCMonth() + 1;
        var yy = date.getUTCFullYear();
        value = (dd < 10 ? "0" : "") + dd + "." + (mm < 10 ? "0" : "") + mm + "." + yy;

        var h = date.getUTCHours();
        var m = date.getUTCMinutes();
        var s = date.getUTCSeconds();
        value += " " + (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "") + m
                 + ":" + (s < 10 ? "0" : "") + s;
        break;

    case "days":

        if (value > (60 * 60 * 24))
        {
            value = Math.ceil(value / 60 / 60 / 24);
            value += " days";
        }
        else if (value > (60 * 60))
        {
            value = Math.ceil(value / 60 / 60);
            value += " hours";
        }
        else if (value > 60)
        {
            value = Math.ceil(value / 60);
            value += " minutes";
        }
        else
        {
            value = Math.ceil(value);
            value += " seconds";
        }

        break;

    case "exclusion":
        var k;
        var c_val = "";
        var text = ""; 

        for (k = 0; k < value.length; k++)
        {
            if (value.charCodeAt(k) != 0)
                c_val += value.charAt(k);
            else
            {
            	 if (c_val) text += c_val + " [<a href=\"javascript:removeexclusion('"+c_val+"');\">remove</a>]<br>";
                c_val = "";

                if ((k + 1) < value.length && value.charCodeAt(k + 1) == 0)
                    break;
            }
        }
        if (text) text+="<br>";
        text += "<a href=\"javascript:runfunction(1,'"+id.split(",")[0]+"','f');\"><img title=\"Delete Volume\" alt=\"Delete Volume\" border=\"0\" src=\"images/delete.gif\"></a> <a href=\"javascript:runfunction(2,'"+id.split(",")[0]+"','f');\"><img title=\"Commit changes\" alt=\"Commit changes\" border=\"0\" src=\"images/CHECK.gif\"></a><br>";

        value = text;
        break;

    case "GetIpFrom1":
        temp = value;
        break;

    case "GetIpFrom2":

        if (is_true(temp))
            value = "Dhcp";
        else if (is_true(value))
            value = "AutoIp Pool";
        else
            value = "StaticIp Pool";

        break;

    case "line":
        var subindex = id.substring(1, id.indexOf(","));

        value = "<div align=\"center\"><input onClick=\"change_line('"
                + (mod.index + 3) + "','" + subindex + "','" + value + "')\" "
                + (value == 1 ? "checked=\"checked\"" : "")
                + " type=\"checkbox\"></div>";

        break;

    case "memdiv":
        var total = document.getElementById("v1," + (mod.index + 1)).innerHTML.replace(" MB", "");
        var used = total - document.getElementById("v2," + (mod.index + 1)).innerHTML.replace(" MB", "");
        var stotal = document.getElementById("v3," + (mod.index + 1)).innerHTML.replace(" MB", "");
        var sused = stotal - document.getElementById("v4," + (mod.index + 1)).innerHTML.replace(" MB", "");

        if (value >= 0 && total >= 0 && used >= 0 && stotal >= 0 && sused >= 0)
        {
            var per = value;

            var mp = Math.ceil(((used / total) * 100) * ((100 - per) / 100));
            var sp = Math.ceil((((sused) / stotal) * 100) * (per / 100));

            pic = "";
            pic += "<img src=\"images/unused.gif\" height=\"12\" width=\"" + 3
                   * sp + "\">";

            if (document.images)
            {
                pre_bar = new Image();
                pre_sli = new Image();
                pre_unu = new Image();
                pre_bar.src = "images/bar.gif";
                pre_sli.src = "images/slider.gif";
                pre_unu.src = "images/unused.gif";
            }

            for ( var k = 1; k < per - sp + 1; k++)
            {
                pic += "<a href=\"javascript:change_memdiv('"+k+"')\"><img alt=\""
                       + k
                       + "%\" src=\"images/bar.gif\" height=\"12\" width=\"3\" border=0></a>";
            }

            pic += "<img alt=\""
                   + per
                   + "%\" src=\"images/slider.gif\" height=\"12\" width=\"4\">";

            for ( var k = 2; k < (100 - per - mp); k++)
            {
                pic += "<a href=\"javascript:change_memdiv('"+(per - sp + k)+"')\"><img alt=\""
                       + (per - sp + k)
                       + "%\" src=\"images/bar.gif\" height=\"12\" width=\"3\" border=0></a>";
            }

            pic += "<img src=\"images/unused.gif\" height=\"12\" width=\"" + 3
                   * mp + "\">";

            value = pic;
            i = -1;
        }

        break;

    default:
        value = value.replace(/^\s*|\s*$/g, "");
    }

    if (i != -1)
    {
        if (value == "")
            value = "<i>-</i>";

        value = "<span style=\"width:90%; display: block;\" onclick=\"edit('"
                + i + "',this.innerHTML,'" + id + "')\">" + value + "</span>";
    }

    return value;
}

function show_customer_pages(name, url)
{
    var text = "<h2>Customer Pages</b><br></h2>";

    if (url != "" && url != undefined)
        text = "<iframe width=\"100%\" frameborder=\"0\" height=\""
               + (screen.availHeight - 280)
               + "\" id='customerpageframe' name='customerpageframe' src='"
               + url + "'></iframe>";

    if (url == "" || url == undefined)
        text += "<p><b>Add Customer Page</b><br />" + addcp_gettext() + "</p>";
    else
        text += "<p>Options: <a href=\"javascript:delete_ds('CP."
                + name
                + "', "
                + (systemdsindex + 2)
                + ", false, '')\"><img border=\"0\" src=\"images/delete.gif\"></a> <a id=\"cpaddbutton\" href=\"javascript:addcp_open()\"><img border=\"0\" src=\"images/ADDFAV.gif\"></a></p>";

    document.getElementById("current").innerHTML = text;
}

function addcp_gettext()
{
    text = "<p><table class=\"datatable\" width=\"90%\" cellpadding=\"1\" cellspacing=\"1\" align=\"center\">";
    text += "<tr><td class=\"titletable\" width=\"40%\" valign=\"top\"><b>Name</b></td><td class=\"normaltable\" width=\"60%\"><input class=\"input\" type=\"text\" id=\"cpaddname\" value=\"\" style=\"width: 98%;\"></td></tr>";
    text += "<tr><td class=\"titletable\" width=\"40%\" valign=\"top\"><b>URL</b></td><td class=\"normaltable\" width=\"60%\"><input class=\"input\" type=\"text\" id=\"cpaddvalue\" value=\"\" style=\"width: 98%;\"></td></tr>";
    text += "<td colspan=\"2\" class=\"normaltable\" align=\"center\"><input class=\"button\" type=\"submit\" onclick=\"javascript:cp_addvar()\" value=\"Add\"></td></tr></table></p>";
    return text;
}

function addcp_open()
{
    text = addcp_gettext();
    var a = getPosition(document.getElementById("cpaddbutton"));
    box_open("Add Customer Page", text, a.x, a.y, 300);
}

function cp_addvar()
{
    var name = document.getElementById("cpaddname").value;

    if (name == "")
        return;

    var rvalue = document.getElementById("cpaddvalue").value;

    if (rvalue == "")
        return;

    var rname = "CP." + name;

    nlen = rname.length;

    vlen = rvalue.length;

    data = [];

    data = prepare_data(nlen + vlen + 12, "int", 4, data);

    data = prepare_data(nlen, "int", 4, data);

    data = prepare_data(vlen, "int", 4, data);

    data = prepare_data(rname, "string", 4, data);

    data = prepare_data(rvalue, "string", 4, data);

    new Mutex (query_readwrite, new QueryData(systemdsindex + 1, 1, 2, data.length, "data", data, "", 0));

    mod_list[mod_count++] = new Module(name, "Customer Pages", rvalue, 513);

    mod_list[mod_count - 1].url = rvalue;

    draw_menu();

    box_close();
}

function show_overview()
{
    var text = "<img src=\"images/cx1000.jpg\"><h2>General Information</b><br></h2>";

    text += "<table cellspacing=\"1\" width=\"90%\" class=\"datatable\" align=\"center\"><tr><td class=\"titletable\" width=\"33%\">Hostname</td><td class=\"normaltable\" id=\"Hostname\"><span style=\"width:90%; display: block;\" onclick=\"edit('-1',this.innerHTML,'Hostname')\">"
            + general_info.hostname
            + "</span></td><td align=\"center\" class=\"normaltable\" width=\"10%\"><input class=\"button\" type=\"submit\" onclick=\"change_hostname()\" id=\"apply\" name=\"apply\" value=\"Apply\"></td></tr>";

    if (general_info.hardware != null)
        text += "<tr><td class=\"titletable\" width=\"33%\">Hardware Version</td><td colspan=\"2\" class=\"lockedtable\">"
                + general_info.hardware + "</td></tr>";

	if (general_info.os != null)
    text += "<tr><td class=\"titletable\" width=\"33%\">OS and Image Version</td><td colspan=\"2\" class=\"lockedtable\">"
            + general_info.os + "</td></tr>";

    text += "<tr><td class=\"titletable\" width=\"33%\">Website Version</td><td colspan=\"2\" class=\"lockedtable\">"
            + version + "</td></tr></table>";

    document.getElementById("current").innerHTML = text;
}

/** *********** EDIT ************** */
function reset_open(id)
/* ie6 onblur */
{

    for ( var i = 0; i < is_open.length; i++)
    {
        if (is_open[i].id == id)
        {
            cut1 = is_open.slice(0, i - 1)
                   cut2 = is_open.slice(i + 1, is_open.length)
                          is_open = cut1 + cut2;
            break;
        }
    }
}

function close_open_fields()
{
    for ( var i = 0; i < is_open.length; i++)
    {
        edit_leave(is_open[i].i, is_open[i].val, is_open[i].id);
    }

    is_open= [];
}

// i=mod.variable index or -1, id=id of the input box
function edit(i, value, id)
{
    var oldvalue = value;
	
    var elem;
    var type;

    if (i == -1)
    {
        type = "";
    }
    else if (id == "plceditbox" || id == "faveditbox")
    {
        type = document.getElementById((id == "faveditbox"?"f":"")+"plctype" + i).innerHTML;
    }
    else
    {
        close_open_fields();
        is_open.push(new editItem(i, value, id));
        type = mod.variables[i].displaytype;
    }

    elem = document.getElementById(id);

    if (value.toLowerCase() == "<i>-</i>")
        value = "";

    text = "";

    switch (type)
    {

    case "yn":
        text = "<select class=\"input\" onchange=\"edit_leave('" + i
               + "', this.options[this.options.selectedIndex].text,'" + id
               + "')\"><option value=\"1\"";

        if (value == "Yes")
            text += "selected";

        text += ">Yes</option><option value=\"0\"";

        if (value == "No")
            text += "selected";

        text += ">No</option></select>";

        break;

    case "BOOL":
        // PLC Type
    case "bool":

    case "tf":
        text = "<select class=\"input\" onchange=\"edit_leave('" + i
               + "', this.options[this.options.selectedIndex].text,'" + id
               + "')\"><option value=\"1\"";

        if (value == "True" || value == "true")
            text += "selected";

        text += ">True</option><option value=\"0\"";

        if (value == "False" || value == "false")
            text += "selected";

        text += ">False</option></select>";

        break;

    case "ed":
        text = "<select class=\"input\" onchange=\"edit_leave('" + i
               + "', this.options[this.options.selectedIndex].text,'" + id
               + "')\"><option value=\"1\"";

        if (value == "Enabled")
            text += "selected";

        text += ">Enabled</option><option value=\"0\"";

        if (value == "Disabled")
            text += "selected";

        text += ">Disabled</option></select>";

        break;

    case "security":
        text = "<select class=\"input\" onchange=\"edit_leave('" + i
               + "', this.options[this.options.selectedIndex].text,'" + id
               + "')\"><option value=\"0\"";

        if (value == "Disabled (0)")
            text += "selected";

        text += ">Disabled (0)</option><option value=\"1\"";

        if (value == "Webpage (1)")
            text += "selected";

        text += ">Webpage (1)</option><option value=\"2\"";

        if (value == "Device (2)")
            text += "selected";

        text += ">Device (2)</option></select>";

        break;

    case "days":
        text = create_selectbox(1, 24, "", " hour", true, "", false, value, i,
                                false, id);

        text = create_selectbox(1, 32, "", " day", true, text, true, value, i,
                                false, id);

        break;

    case "ewf-bootcommand":
				
				text = "<select class=\"input\" onchange=\"edit_leave('" + i + "', this.options[this.options.selectedIndex].text,'" + id + "')\">";
						 
				text += "<option value=\"Clear Command\" ";
				if ( value == "NoCommand" ) text += "selected";
				text += ">Clear Command</option>";
						 
				text += "<option value=\"Enable\" ";
				if ( value == "Enable" ) text += "selected";
				text += ">Enable</option>";
						 
				text += "<option value=\"Disable\" ";
				if ( value == "Disable" ) text += "selected";
				text += ">Disable</option>";
						 
				text += "<option value=\"Disable\" ";
				text += ">Disable / Commit</option>";
						 
				text += "<option value=\"Commit\" ";
				if ( value == "Commit" ) text += "selected";
				text += ">Commit</option>";
						 
				text += "</select>";
 				break;
		
		
		case "resolution":

    case "timezone":
        text = "<select class=\"input\" onchange=\"edit_leave('" + i
               + "', this.value,'" + id + "')\" style=\"width:95%\">";

        for ( var j = 0; j < currenttemp.length; j++)
        {		
            text += "<option value=\"" + currenttemp[j] + "\"";

            if (currenttemp[j] == value)
                text += "selected";

            text += ">" + currenttemp[j] + "</option>";
        }

        text += "</select>";
        break;
				
				

    case "getipfrom1":

    case "getipfrom2":
        text = "<select class=\"input\" onchange=\"edit_leave('" + i
               + "', this.value,'" + id + "')\"><option value=\"Dhcp\"";

        if (value == "Dhcp")
            text += "selected";

        text += ">Dhcp</option><option value=\"AutoIp Pool\"";

        if (value == "AutoIp Pool")
            text += "selected";

        text += ">AutoIp Pool</option><option value=\"StaticIp Pool\"";

        if (value == "StaticIp Pool")
            text += "selected";

        text += ">StaticIp Pool</option></select>";

        break;

    case "time":
        var dtarr = value.split(" ");

        var timearr = dtarr[1].split(":");

        // Convert utc time to locale time for calendar gui
        var utctime = new Date();

        utctime.setUTCHours(parseFloat(timearr[0]));

        utctime.setUTCMinutes(parseFloat(timearr[1]));

        utctime.setUTCSeconds(parseFloat(timearr[2]));

        var h = utctime.getHours();

        var m = utctime.getMinutes();

        var s = utctime.getSeconds();

        var localeTimeStr = (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "")
                            + m + ":" + (s < 10 ? "0" : "") + s;

        var localeTimeArr = localeTimeStr.split(":");

        text += date_picker_refresh(dtarr[0], localeTimeArr, i, id);

        break;

    case "dir":
        if (winxp && value.charAt(1) != ":")
            value = "C:" + value;

        text = "<input id=\"selectedfolder\" class=\"input\" type=\"text\" style=\"width:98%;\" value=\""
               + value
               + "\"><input id=\"selectedfolderid\" type=\"hidden\" value=\""
               + id
               + "\"><br><div id=\"subfolders\">loading subfolders...</div>";

        text += "<input class=\"button\" type=\"submit\" onclick=\"folder_leave('"
                + i + "',false)\" value=\"close\">";

        window.setTimeout("folderchange('" + double_slashes(value)
                          + "','','DIR','');", 200);

        break;

    case "plcvar":
        text = "<input id=\"selectedvar\" class=\"input\" type=\"text\" style=\"width:98%;\" value=\""
               + value
               + "\"><input id=\"selectedvarid\" type=\"hidden\" value=\""
               + id + "\"><br><div id=\"subfolders\">loading...</div>";

        text += "<input class=\"button\" type=\"submit\" onclick=\"folder_leave('"
                + i + "',true)\" value=\"close\">";

        window.setTimeout("folderchange('" + value + "','','PLC','');", 200);

        break;

    default:
        text = "<input class=\"input\" type=\"text\" style=\"width:98%;\" onblur=\"edit_leave('"
               + i + "', this.value,'" + id + "')\" value=\"" + value + "\">";

        break;
    }

    elem.innerHTML = text;

    elem.firstChild.focus();
    elem.firstChild.focus();
}

function folderchange(folder, parent, type, qid)
{
    new Mutex(doFolderchange, new FolderData(folder, parent, type, qid),0);
}

function doFolderchange(folderData)
{
    if (folderData.folder == null)
    {
        if (folderData.type == "DIR")
            folderData.folder = "\\";
        else
            folderData.folder = "";
    }

    if (folderData.parent == null)
        folderData.parent = "";

    if (folderData.folder.charAt(folderData.folder.length - 1) != "\\" && folderData.type == "DIR")
        folderData.folder += "\\";

    if (folderData.parent)
    {
        if (folderData.type == "DIR")
        {
            if (folderData.parent.charAt(folderData.parent.length - 1) != "\\")
                folderData.parent += "\\";
        }
        else
        {
            if (folderData.parent.charAt(folderData.parent.length - 1) != "."
                    && folderData.folder.charAt(0) != "[")
                folderData.parent += ".";
        }

        folderData.folder = folderData.parent + folderData.folder;
    }

    if (folderData.type == "DIR" && (folderData.folder.charAt(0) != "\\" && folderData.folder.charAt(1) != ":"))
        folderData.folder = "\\" + folderData.folder;

    var id = "";

    var idx = -1;

    if (folderData.type == "DIR")
    {
        idx = folderindex;
        id = "selectedfolder";
    }
    else
    {
        idx = bidx;
        id = "selectedvar";
    }

    try
    {
        document.getElementById(id).value = folderData.folder;
    }
    catch (e)
    {
    }

    var ndata = [];

    if (folderData.type == "PLC")
    {
    	  load_start();
        ndata = prepare_data(0, "int", 4, ndata);
        // version
        ndata = prepare_data(20 + folderData.folder.length + 1, "int", 4, ndata);
        // cbData
        ndata = prepare_data(0, "int", 4, ndata);
        // hContinue
        ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
        // maxReadLen
        // ndata = prepare_data(31,"int",4,ndata); //maxReadLen
        ndata = prepare_data(folderData.folder.length + 1, "int", 4, ndata);
        // cbPath
        ndata = prepare_data(folderData.folder, "string", 0, ndata);
        // szPath
        full_buffer = [];
        full_buffer_param = folderData.folder;
    }
    else if (folderData.type == "DIR")
    {
        ndata = prepare_data(folderData.folder.length + 1, "int", 4, ndata);
        ndata = prepare_data(folderData.folder + "*", "string", 0, ndata);
    }
    else
    {
        ndata = prepare_data(16 + folderData.folder.length + 1, "int", 4, ndata);
        // cbData
        ndata = prepare_data(0, "int", 4, ndata);
        // hContinue
        ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
        // maxReadLen
        ndata = prepare_data(folderData.folder.length + 1, "int", 4, ndata);
        // cbPath
        ndata = prepare_data(folderData.folder, "string", 0, ndata);
        full_buffer = [];
        full_buffer_param = folderData.folder;
    }

    new Mutex (query_readwrite, new QueryData(idx, 1, 2, buffer_size, "data", ndata, folderData.qid, false),0);
    folderparent = folderData.folder;

    document.getElementById(document.getElementById(id + "id").value).value = folderData.folder;

    if (folderData.mutexIndex)
        Mutex.finish(folderData.mutexIndex);
}

function edit_leave(i, value, id)
{
    reset_open(id);
		
    try
    {
        oldvalue = document.getElementById(id + "old");
				
        if (oldvalue)
        {		
            oldvalue = oldvalue.value;
						
            if (value != oldvalue)
            {	
								
                document.getElementById(id + "info").style.display = "";
                document.getElementById("apply").disabled = false;
                document.getElementById("apply").className = "button";
                apply_button_enable(true);
            }
            else
            {	
                document.getElementById(id + "info").style.display = "none";

                if (!check_var_changed(id))
                {
                    apply_button_enable(false);
                }
            }
        }

        if ((id != "plceditbox" && id != "faveditbox") && i > 0 && mod.variables[i].need == -2)
        {
            check_var_disable(i, is_true(value));
            do_var_disable();
        }
    }
    catch (e)
    {
    }

    if (value == "")
        value = "<i>-</i>";

    document.getElementById(id).innerHTML = "<span style=\"width:100%;\" onclick=\"edit('"+ i + "',this.innerHTML,'" + id + "')\">" + value + "</span>";

    if (id == "plceditbox" || id == "faveditbox")
        // write w/o extra apply on plcedit
    {
        write_plc_val(document.getElementById((id == "faveditbox"?"f":"")+"plcname" + i).innerHTML, value, document.getElementById((id == "faveditbox"?"f":"")+"plcsize" + i).innerHTML, document.getElementById((id == "faveditbox"?"f":"")+"plctype" + i).innerHTML);
        document.getElementById((id == "faveditbox"?"f":"")+"plcvar" + i).innerHTML = value;
    }
}

function apply_button_enable(enable)
{
    if (enable)
    {
        document.getElementById("apply").disabled = false;
        document.getElementById("apply").className = "button";
    }
    else
    {
        document.getElementById("apply").disabled = true;
        document.getElementById("apply").className = "button_disabled";
    }
}

function time_leave(i, id, day)
{
    // Convert local time to UTC time to compare it with old value
    var localetime = new Date();
    localetime.setHours(parseFloat(document.getElementById("timeHour").value));
    localetime.setMinutes(parseFloat(document.getElementById("timeMinute").value));
    localetime.setSeconds(parseFloat(document.getElementById("timeSecond").value));

    var h = localetime.getUTCHours();
    var m = localetime.getUTCMinutes();
    var s = localetime.getUTCSeconds();
    var timestring = (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s;

    var ms = (parseFloat(document.getElementById("selMonth").value, 10) + 1).toString();

    if (ms.length == 1)
        ms = "0" + ms;

    if (day < 10)
        day = "0" + day;

    var datestring = day + "." + ms + "." + document.getElementById("selYear").value;

    edit_leave(i, datestring + " " + timestring, id);
}

function date_picker_change(i, id)
{
    var ms = (parseFloat(document.getElementById("selMonth").value, 10) + 1).toString();

    if (ms.length == 1)
        ms = "0" + ms;

    timearr = new Array(document.getElementById("timeHour").value, document.getElementById("timeMinute").value, document.getElementById("timeSecond").value);

    document.getElementById("v" + mod.variables[i].subindex + "," + (mod.index + 1)).innerHTML = date_picker_refresh("00." + ms + "." + document.getElementById("selYear").value, timearr, i, id);
}

function date_picker_refresh(datestring, timearr, i, id)
{
    day = parseFloat(datestring.charAt(0) + datestring.charAt(1), 10);
    month = parseFloat(datestring.charAt(3) + datestring.charAt(4), 10) - 1;
    year = parseFloat(datestring.charAt(6) + datestring.charAt(7)
                      + datestring.charAt(8) + datestring.charAt(9), 10);

    var dayArray = new Array('S', 'M', 'T', 'W', 'T', 'F', 'S');
    var monthArray = new Array('January', 'February', 'March', 'April', 'May',
                               'June', 'July', 'August', 'September', 'October', 'November',
                               'December');
    var date = new Date();

    if (month >= 0 && year > 0)
    {
        date = new Date(year, month, 1);
    }
    else
    {
        day = date.getDate();
        date.setDate(1);
    }

    year = date.getYear();

    if (year < 1900)
        year = year + 1900;

    // month + year
    datetable = "<table width=\"300\"><tr><td colspan=\"7\"><strong>Enter your local time, it will be converted to UTC.</strong><br><br></td></tr><tr><td colspan=\"7\"><select style=\"width:40%;\" id=\"selMonth\" onChange=\"date_picker_change("
                + i + ",'" + id + "')\">";

    for ( var j = 0; j < monthArray.length; j++)
    {
        datetable += "<option value=\"" + j + "\"";

        if (date.getMonth() == j)
            datetable += " selected";

        datetable += ">" + monthArray[j] + "</option>";
    }

    datetable += "</select><input id=\"selYear\" class=\"input\" type=\"text\" style=\"width:40%;\" value=\""
                 + year
                 + "\"> <input class=\"button\" type=\"submit\" onclick=\"date_picker_change('"
                 + i + "','" + id + "')\" value=\"Go\"></td></tr><tr>";

    for ( var j = 0; j < dayArray.length; j++)
    {
        datetable += "<td class=\"toptable\" align=\"center\"><b>"
                     + dayArray[j] + "</b></td>";
    }

    datetable += "</tr><tr>";

    // Convert local time to UTC time to compare it with old value
    var localetime = new Date();
    localetime.setHours(parseFloat(timearr[0]));
    localetime.setMinutes(parseFloat(timearr[1]));
    localetime.setSeconds(parseFloat(timearr[2]));

    var h = localetime.getUTCHours();
    var m = localetime.getUTCMinutes();
    var s = localetime.getUTCSeconds();
    var timestring = (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "") + m
                     + ":" + (s < 10 ? "0" : "") + s;

    for (j = 0; j < date.getDay(); j++)
        datetable += "<td class=\"lockedtable\">&nbsp;</td>";

    do
    {
        curDay = date.getDate();

        var cssclass = "normaltable";

        if (curDay == day)
            cssclass = "titletable";

        var ds = curDay.toString();

        if (ds.length == 1)
            ds = "0" + ds;

        var ms = (date.getMonth() + 1).toString();

        if (ms.length == 1)
            ms = "0" + ms;

        var new_datestring = ds + "." + ms + "." + year;

        datetable += "<td class=\"" + cssclass
                     + "\" align=\"center\"><a href=\"javascript:\edit_leave(" + i
                     + ",'" + new_datestring + " " + timestring + "','" + id
                     + "')\">" + ds + "</a></td>";

        if (date.getDay() == 6)
            datetable += "</tr><tr>";

        date.setDate(date.getDate() + 1);
    }
    while (date.getDate() > 1)

        if (date.getDay() > 0)
        {
            for (j = 6; j >= date.getDay(); j--)
                datetable += "<td class=\"lockedtable\">&nbsp;</td>";
        }

    datetable += "</tr><tr><td colspan=\"7\"><b>Time:</b> ";

    datetable += create_selectbox(0, 23, "timeHour", "", false, "", true,
                                  timearr[0], -1, true, id)
                 + " ";
    datetable += create_selectbox(0, 59, "timeMinute", "", false, "", true,
                                  timearr[1], -1, true, id)
                 + " ";
    datetable += create_selectbox(0, 59, "timeSecond", "", false, "", true,
                                  timearr[2], -1, true, id)
                 + " ";
    datetable += "<input class=\"button\" type=\"submit\" onclick=\"time_leave('"
                 + i + "','" + id + "', '" + day + "')\" value=\"Change\">";

    datetable += "</td></tr></table>";

    return datetable;
}

/** ********* EDIT END ************ */

function create_selectbox(min, max, id, append, appendS, appendOptions,
                          appendSelect, selected, i, leadingzero, oid)
{
    var text = "";

    if (appendSelect)
    {
        text = "<select class=\"input\"";

        if (id != "")
            text += "id=\"" + id + "\"";

        if (i != -1)
            text += " onchange=\"edit_leave('" + i + "', this.value,'" + oid
                    + "')\"";

        text += ">";
    }

    if (appendOptions != "")
        text += appendOptions;

    for ( var j = min; j <= max; j++)
    {
        if (leadingzero && j < 10)
            val = "0" + j.toString();
        else
            val = j;

        text += "<option value=\"" + val + append
                + (appendS ? (j == 1 ? "" : "s") : "") + "\"";

        if (val + append + (appendS ? (j == 1 ? "" : "s") : "") == selected)
                    text += "selected";

        text += ">" + val + append + (appendS ? (j == 1 ? "" : "s") : "")
                        + "</option>";
    }

    if (appendSelect)
{
        text += "</select>"
            }

            return text;
}

/** ***********APPLY************** */
function apply(subindex)
{		
    clear_info();
    close_open_fields();
    datarray = [];
    sendnamearray = [];

    k = 0;

    var idx = [];
    var sidx = [];
    var flag = [];
    var len = [];
    var type = [];
    var data = [];

    var offset = 0;
		
		
		
    for ( var i = 0; i < mod.variables.length; i++)
    {		
        if (mod.variables[i].displaytype == "hidden" || mod.variables[i].editable==false)
            continue;
				
        var value = "";

        try
        {
					if (subindex == -1)
					{	
            value = document.getElementById("v"
                                            + mod.variables[i].subindex
                                            + ","
                                            + (mod.variables[i].index == 0 ? (mod.index + 1)
                                               : mod.variables[i].index)).firstChild.innerHTML;
					}
					else if (mod.variables[i].editable)
					{

						value = document.getElementById("a"
                                            + subindex
                                            + ","
                                            + (mod.variables[i].index == 0 ? (mod.index + 1)
                                             : mod.variables[i].index)).firstChild.innerHTML;
					
					}
					

            if (!value)
                value = "";
        }
        catch (e)
        {
            show_info("Error: Unexpected error", "red");
            value = "";
        }

				if (value.toLowerCase() == "<i>-</i>")
            value = "";

        switch (mod.variables[i].displaytype)
        {

        case "yn":

            if (value == "Yes")
                value = 1;
            else
                value = 0;

            break;

        case "bool":

        case "tf":
            if (value == "True")
                value = 1;
            else
                value = 0;

            break;

        case "ed":
            if (value == "Enabled")
                value = 1;
            else
                value = 0;

            break;

        case "security":
            if (value == "Disabled (0)")
                value = 0;
            else if (value == "Webpage (1)")
                value = 1;
            else
                value = 2;

            break;

        case "days":
            value = value.split(' ');

            if (value[1].startsWith("day"))
                value = value[0] * 60 * 60 * 24;
            else
                value = value[0] * 60 * 60;
            break;					
				
				
				case "ewf-bootcommand":
						if(value == "Clear Command")
							value = 0;
						else if(value == "Enable")
							value = 1;
						else if(value == "Disable")
							value = 2;
						else if(value == "Disable / Commit")
							value = 3;
						else if(value == "Commit")
							value = 4;
						else
							value = 0;
						break;
				
        case "rf":
            // not editable
            break;

        case "GetIpFrom1":
            value = document.getElementById(params[i + 1].name).firstChild.innerHTML;

            if (value == "Dhcp")
                value = 1;
            else
                value = 0;

            break;

        case "GetIpFrom2":
            if (value == "AutoIp Pool")
                value = 1;
            else
                value = 0;

            break;

        case "time":
            var dtarr = value.split(" ");

            var datearr = dtarr[0].split(".");

            var timearr = dtarr[1].split(":");

            var date = new Date();

            date.setUTCDate(parseFloat(datearr[0]));

            date.setUTCMonth(parseFloat(datearr[1]) - 1);

            date.setUTCFullYear(parseFloat(datearr[2]));

            date.setUTCHours(parseFloat(timearr[0]));

            date.setUTCMinutes(parseFloat(timearr[1]));

            date.setUTCSeconds(parseFloat(timearr[2]));

            value = date.valueOf() / 1000;

            break;

        case "resolution":
            var offset = 1;

        case "timezone":
            for ( var j = 0; j < currenttemp.length; j++)
            {
                if (currenttemp[j] == value)
                {
                    value = j + offset;
                    break;
                }
            }
            break;

        case "netid":

        default:
            break;
        }

        if (mod.variables[i].editable)
        {
            idx.push(mod.variables[i].index);
            sidx.push(mod.variables[i].subindex);
            len.push(mod.variables[i].len);
            type.push(mod.variables[i].datatype);
            data.push(value);
        }
    }
		
    new Mutex(query_writemultiple,new QueryData(idx, sidx, 0, len, type, data, "apply", false),0);
}

function clear_info()
{
    document.getElementById("infotext").style.display = "none";
    document.getElementById("infotext").innerHTML = "";
}

function show_info(text, color)
{
    document.getElementById("infotext").style.display = "";
    document.getElementById("infotext").innerHTML += "<span style=\"color: "
            + color + ";\">" + text + "</span><br />";
}

function debug(text)
{
    debug(text, false);
}

function debug(text, replace)
{
    if (g_debug)
    {
        if (replace)
        {
            text = text.replace(/</g, "&lt;");
            text = text.replace(/>/g, "&gt;");
        }

        if (document.getElementById("debug").innerHTML.length > g_debug_max_size)
            document.getElementById("debug").innerHTML = "";

        document.getElementById("debug").innerHTML += text;
    }
}

function debugln(text)
{
    debug(text + "<br>");
}

function MM_findObj(n, d)
{
    // v3.0
    var p, i, x;

    if (!d)
        d = document;

    if ((p = n.indexOf("?")) > 0 && parent.frames.length)
    {
        d = parent.frames[n.substring(p + 1)].document;
        n = n.substring(0, p);
    }

    if (!(x = d[n]) && d.all)
        x = d.all[n];

    for (i = 0; !x && i < d.forms.length; i++)
        x = d.forms[i][n];

    for (i = 0; !x && d.layers && i < d.layers.length; i++)
        x = MM_findObj(n, d.layers[i].document);

    return x;
}

/* Functions that swaps down images. */
function MM_nbGroup(event, grpName)
{
    // v3.0
    var i, img, nbArr, args = MM_nbGroup.arguments;

    if (event == "init" && args.length > 2)
    {
        if ((img = MM_findObj(args[2])) != null && !img.MM_init)
        {
            img.MM_init = true;
            img.MM_up = args[3];
            img.MM_dn = img.src;

            if ((nbArr = document[grpName]) == null)
                nbArr = document[grpName] = [];

            nbArr[nbArr.length] = img;

            for (i = 4; i < args.length - 1; i += 2)
                if ((img = MM_findObj(args[i])) != null)
                {
                    if (!img.MM_up)
                        img.MM_up = img.src;

                    img.src = img.MM_dn = args[i + 1];

                    nbArr[nbArr.length] = img;
                }
        }
    }
    else if (event == "over")
    {
        document.MM_nbOver = nbArr = [];

        for (i = 1; i < args.length - 1; i += 3)
            if ((img = MM_findObj(args[i])) != null)
            {
                if (!img.MM_up)
                    img.MM_up = img.src;

                img.src = (img.MM_dn && args[i + 2]) ? args[i + 2]
                          : args[i + 1];

                nbArr[nbArr.length] = img;
            }
    }
    else if (event == "out")
    {
        for (i = 0; i < document.MM_nbOver.length; i++)
        {
            img = document.MM_nbOver[i];
            img.src = (img.MM_dn) ? img.MM_dn : img.MM_up;
        }
    }
    else if (event == "down")
    {
        if ((nbArr = document[grpName]) != null)
            for (i = 0; i < nbArr.length; i++)
            {
                img = nbArr[i];
                img.src = img.MM_up;
                img.MM_dn = 0;
            }

        document[grpName] = nbArr = [];

        for (i = 2; i < args.length - 1; i += 2)
            if ((img = MM_findObj(args[i])) != null)
            {
                if (!img.MM_up)
                    img.MM_up = img.src;

                img.src = img.MM_dn = args[i + 1];

                nbArr[nbArr.length] = img;
            }
    }
}

var boxobj;
var overobj = false;

function mouse_down(e)
{
    if (overbox)
    {   
	 	  if (window.event) e=window.event;
 
    	  boxobj = document.getElementById("box");
    	  
			var dragX = parseInt(boxobj.style.left);
			var dragY = parseInt(boxobj.style.top);
			
			var mouseX = e.clientX;
			var mouseY = e.clientY;

    	  	X = mouseX - dragX;
			Y = mouseY - dragY;

        /*if (!document.is_ie)
        {
            X = e.layerX;
            Y = e.layerY;
            return false;
        }
        else
        {
            X = event.offsetX;
            Y = event.offsetY;
        } */
    }

    return true;
}

function mouse_move(e)
{
    if (boxobj)
    {
        /*if (!document.is_ie)
        {
            boxobj.style.top = (e.pageY - Y) + "px";
            boxobj.style.left = (e.pageX - X) + "px";
        }
        else
        {
            boxobj.style.top = (event.clientY - Y) + "px";
            boxobj.style.left = (event.clientX - X) + "px";
            return false;
        }*/
        
        if (window.event) e=window.event;
		
	var newX = e.clientX - X;
	var newY = e.clientY - Y;

	boxobj.style.left = newX + "px";
	boxobj.style.top = newY + "px";

    }

    return true;
}

function mouse_up()
{
    boxobj = null;
}

function box_open(title, text, x, y, width)
{
    document.getElementById("box").style.display = "";
    document.getElementById("box").style.top = y + "px";
    document.getElementById("box").style.left = x + "px";
    document.getElementById("box").style.width = width;
    document.getElementById("box_title").innerHTML = title;
    document.getElementById("box_text").innerHTML = text;
}

function box_close()
{
    document.getElementById("box").style.display = "none";
}

function sortMenu(a, b)
{
    return a.order - b.order;
}

function draw_menu()
{
    // Sortiere mod_list by order
    mod_list = mod_list.sort(sortMenu);

    var showRegFilter = false;

    for ( var i = 0; i < mod_count; i++)
    {
        // Show Regfilter only if EWF or FBWF exist.
        if (mod_list[i].type == "EWF" && mod_list[i].hide == false
                || mod_list[i].type == "FBWF" && mod_list[i].hide == false)
            showRegFilter = true;
    }

    var current_list = "";
    var current_customFolder = "";
    var text = "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"1\" id=\"menutable\" class=\"menu\">";

    for ( var i = 0; i < mod_count; i++)
    {
        // Show regfilter only if EWF or FBWF exist
        if (mod_list[i].type == "Regfilter" && !showRegFilter)
            mod_list[i].hide = true;

        // Show element only if is hide is not true
        if (!mod_list[i].hide)
        {
            if (mod_list[i].type == "Customer Pages"
                    && count_types(mod_list[i].type) > 1)
                nameN = mod_list[i].name;
            else if (count_types(mod_list[i].type) > 1)
                nameN = mod_list[i].name;
            else if (mod_list[i].type == "SiliconDrive")
                nameN = "Storage";
            else
                nameN = mod_list[i].type;

            if (mod_list[i].type == "FSO")
                continue;

            // Print folder image
            // Custom folder
            if (mod_list[i].customFolder != "" && current_customFolder != mod_list[i].customFolder)
            {
								// Close folder if a new customfolder follows on a customFolder
								if(current_customFolder != "" && current_customFolder != mod_list[i].customFolder)
								{	
										text += "</table></div></td></tr>";
										current_customFolder = "";
								}
								
								// Close folder if a new customfolder folder follows on a multitype folder
								if(current_list!= "" && current_list != mod_list[i].type)
								{	
										text += "</table></div></td></tr>";
										current_list = "";
								}
								
								text += "<tr><td valign=\"top\"><table width=\"100%\" class=\"menu\" cellspacing=\"0\" cellpadding=\"1\"><tr><td width=\"60\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><a href=javascript:collapse('"
                        + i
                        + 999
                        + "')><img alt=\"\" id=\"micon_"
                        + i
                        + 999
                        + "\" border=\"0\" src=\"images/MU.gif\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></a></td><td>"
                        + mod_list[i].customFolder
                        + "</td></tr></table></td></tr><tr><td valign=\"top\"><div id=\"mdiv_"
                        + i
                        + 999
                        + "\"><table class=\"menu\" width=\"100%\" cellspacing=\"0\" cellpadding=\"1\">";
                
								current_customFolder = mod_list[i].customFolder;
            }

            // Multitype folder
            if ((count_types(mod_list[i].type) > 1 && current_list != mod_list[i].type))
            {
							 	// Close folder if a new multitype folder follows on a customfolder
								if(current_customFolder != "" && current_customFolder != mod_list[i].customFolder)
								{	
										text += "</table></div></td></tr>";
										current_customFolder = "";
								}
								
								// Close folder if a new multitype folder follows on a multitype folder
								if(current_list!= "" && current_list != mod_list[i].type)
								{	
										text += "</table></div></td></tr>";
										current_list = "";
								}
							 
							 // Multitype folder on level 1
                if (current_customFolder == "" || current_customFolder != mod_list[i].customFolder)
                {
                    text += "<tr><td valign=\"top\"><table width=\"100%\" class=\"menu\" cellspacing=\"0\" cellpadding=\"1\"><tr><td width=\"60\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><a href=javascript:collapse('"
                            + i
                            + "')><img alt=\"\" id=\"micon_"
                            + i
                            + "\" border=\"0\" src=\"images/MU.gif\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></a></td><td>"
                            + mod_list[i].type
                            + "</td></tr></table></td></tr><tr><td valign=\"top\"><div id=\"mdiv_"
                            + i
                            + "\"><table class=\"menu\" width=\"100%\" cellspacing=\"0\" cellpadding=\"1\">";
                }
                else // Multitype folder on level 2
                {
                    text += "<tr><td valign=\"top\"><table width=\"100%\" class=\"menu\" cellspacing=\"0\" cellpadding=\"1\"><tr><td width=\"80\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><a href=javascript:collapse('"
                            + i
                            + "')><img alt=\"\" id=\"micon_"
                            + i
                            + "\" border=\"0\" src=\"images/MU.gif\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></a></td><td>"
                            + mod_list[i].type
                            + "</td></tr></table></td></tr><tr><td valign=\"top\"><div id=\"mdiv_"
                            + i
                            + "\"><table class=\"menu\" width=\"100%\" cellspacing=\"0\" cellpadding=\"1\">";
                }

                current_list = mod_list[i].type;
            }

					  // Close folder if a leaf follows on a multitype fodler
						if(current_list != "" && current_list != mod_list[i].type)
					  {	
								text += "</table></div></td></tr>";
								current_list = "";
						}
						
						 // Close folder if a leaf follows on a custom fodler
						if(current_customFolder != "" && current_customFolder != mod_list[i].customFolder)
					  {	
								text += "</table></div></td></tr>";
								current_customFolder = "";
						}
					 
					  // Print leaf on level > 1
            if ((count_types(mod_list[i].type) > 1 && current_list == mod_list[i].type || mod_list[i].customFolder != "" && current_customFolder == mod_list[i].customFolder))
            {	
							 // leaf on level 3
                if (current_customFolder != "" && current_list == mod_list[i].type && current_customFolder == mod_list[i].customFolder)
                {
                   
										text += "<tr><td valign=\"top\"><table width=\"100%\" cellspacing=\"0\" cellpadding=\"1\" class=\"menu\"><tr><td width=\"100\" id=\"m"
                            + i
                            + "i\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/SE.gif\"></td><td id=\"m"
                            + i
                            + "\"><a href=\""
                            + fixLink("index.htm#" + i)
                            + "\">"
                            + nameN
                            + "</a></td></tr></table></td></tr>";
                }
                else // leaf on level 2
                {	 
										text += "<tr><td valign=\"top\"><table width=\"100%\" cellspacing=\"0\" cellpadding=\"1\" class=\"menu\"><tr><td width=\"80\" id=\"m"
                            + i
                            + "i\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/SE.gif\"></td><td id=\"m"
                            + i
                            + "\"><a href=\""
                            + fixLink("index.htm#" + i)
                            + "\">"
                            + nameN
                            + "</a></td></tr></table></td></tr>";
                }
            }
            else // Print leaf on level 1
            {
                text += "<tr><td valign=\"top\"><table width=\"100%\" class=\"menu\" cellspacing=\"0\" cellpadding=\"1\"><tr><td width=\"60\" id=\"m"
                        + i
                        + "i\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/SE.gif\"></td><td id=\"m"
                        + i
                        + "\"><a href=\""
                        + fixLink("index.htm#" + i)
                        + "\">" + nameN + "</a></td></tr></table></td></tr>";
            }
        }
		}
		
		// Close folder if reboot leaf follows on a multitype fodler
		if(current_list != "")
		{	
				text += "</table></div></td></tr>";
				current_list = "";
		}
		
		 // Close folder if reboot leaf follows on a custom fodler
		if(current_customFolder != "")
		{	
				text += "</table></div></td></tr>";
				current_customFolder = "";
		}
		
    //add reboot button
    text += "<tr><td valign=\"top\"><table width=\"100%\" class=\"menu\" cellspacing=\"0\" cellpadding=\"1\"><tr><td width=\"60\" id=\"m"
            + i
            + "i\" valign=\"top\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/LU.gif\"><img alt=\"\" border=\"0\" src=\"images/SE.gif\"></td><td id=\"m"
            + i
            + "\"><a href=\""
            + "javascript:doReboot(true);"
            + "\">Reboot</a></td></tr></table></td></tr>";

    text += "</table>";

    document.getElementById("mdiv_main").innerHTML = text;
    
    Mutex.notify("menuLoaded");
}

function printPositionASPIframeForIE()
{
    var windowlocator = new Locator("window.location.href", "#");
    document.write("<iframe id=\"ajaxnav\" name=\"ajaxnav\" src=\"position.asp?hash="
                   + windowlocator.getHash()
                   + "\" style=\"display: none;\"></iframe>");
}