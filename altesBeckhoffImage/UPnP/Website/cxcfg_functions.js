function getPosition(obj)
{

    var pos = { x : 0, y : 0 };

    if (obj.offsetParent)
    {
        do
        {
            pos.x += obj.offsetLeft;
            pos.y += obj.offsetTop;
        }
        while ((obj = obj.offsetParent) !== null);
    }
    else if (obj.x)
    {
        pos.x = obj.x;
        pos.y = obj.y;
    }

    return pos;
}

function fixUrls()
{
    if (document.is_ie)
    {
        fix = new AjaxIframesFixer();
    }
    else fix = new AjaxUrlFixer();
}

function write_plc_val(name, value, size, type)
{
    var ndata = [];
    ndata = prepare_data(name.length + value.length + 12, "int", 4, ndata);
    ndata = prepare_data(name.length, "int", 4, ndata);
    ndata = prepare_data(value.length, "int", 4, ndata);
    ndata = prepare_data(name, "string", 0, ndata);
    value = to_plc_val(type, size, value);

    if (value === false) return;
    ndata = prepare_data(value, "string", 0, ndata);

    var qData = new QueryData(browseindex+3, 1, 2, 0, "data", ndata, "SetPlcValue", false);
    
    new Mutex(query_write,qData,0);         
}

function doReboot(need_confirm)
{
    var answer;
    if (need_confirm)
    {
        answer = confirm("Do you really want to reboot the device?");
    }
    else answer = 1;

    if (answer != 0)
    {
        if (rebootindex == -1)
        {
            show_info("Error: Unable to reboot", "red");
            return;
        }

        reboottext = "Rebooting";
        is_rebooting = true;

        var qData = new QueryData(rebootindex, 1, 0, 0, "int", 1, "reboot", true);
        new Mutex(query_write,qData,1);

        if (g_interval)
            window.clearInterval(g_interval);

        g_interval = window.setInterval(RebootLoop, 5000);
    }
}


function rebootWithQuestion(text)
{
    if (confirm(text + " You need to reboot for changes to take effect. Reboot now?"))
    {
        addquery = "";
        doReboot(false, true);
    }
}

function change_memdiv(value)
{
    var qData = new QueryData(mod.index + 1, 5, 0, 0, "int", value, "", false);
    new Mutex(query_write,qData,0);
}

function change_hostname()
{
    var value = document.getElementById("Hostname").firstChild.innerHTML;

    var qData = new QueryData(0x1008, 0, 0, 0, "string", value, "", false);
    qData.refresh = true;
    new Mutex(query_write,qData,0);

    rebootWithQuestion("Settings saved.");
}

function get_dots(count)
{
    var dots = "...", i;

    for (i = 0; i < count; i++)
    {
        dots += ".";
    }

    return dots;
}

function RebootLoop()
{
    if (is_rebooting)
    {
        document.getElementById("current").innerHTML = "<p align=\"center\"><font size=\"+1\">"
                + reboottext
                + "...<br>be patient.<br>"
                + get_dots(rebootcount)
                + "</font></p>";
        rebootcount++;

        var idx, sidx, flag, len, type;
        idx = [];
        idx.push(0x1008);
        sidx = [];
        sidx.push(0);
        flag = [];
        flag.push(0);
        len = [];
        len.push(80);
        type = [];
        type.push("string");
        new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, len, type, [], "reboot", true),1);
    }
}

function loadXMLDoc(url, xmlhttp, func)
{
    xmlhttp.onreadystatechange = function () {};
    xmlhttp.abort();
    xmlhttp.onreadystatechange = func;
    xmlhttp.open("POST", url, true);
}

function createXMLHttpObject()
{
    try
    {
        return new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch (e)
    {
    }

    try
    {
        return new ActiveXObject("Microsoft.XMLHTTP");
    }
    catch (e)
    {
    }

    try
    {
        return new XMLHttpRequest();
    }
    catch (e)
    {
    }

    return null;
}

function cpuloop()
{
    if (mod.type == "CPU")
    {
        var idx, sidx, flag, len, type, i;
        idx = [(mod.index + 1)];
        sidx = [2];
        flag = [0];
        len = [2];
        type = ["int"];
        i = ["1"];

        new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, len, type, i, "CPU", true),1);
    }
}

function raidloop()
{
    if (mod.type == "RAID Controller")
    {
        var idx, sidx, flag, len, type, i, k;
	k = 1;
	idx = [];
	sidx = [];
	flag = [];
	len = [];
	type = [];
	i = [];
        while (document.getElementById("a"+k+","+(mod.index+3)))
	{
	        idx.push(mod.index + 3); //update info
        	sidx.push(k);
	        flag.push(0);
        	len.push(4);
	        type.push("int");
        	i.push("5");
		k++;

		var j = 1;
	        while (document.getElementById("a"+j+","+(mod.index+10)))
		{
		        idx.push(mod.index + 10); //update drive state
	        	sidx.push(j);
		        flag.push(0);
	        	len.push(2);
	        	type.push("int");
        		i.push("3");
			j++;
		}

	}

	k = 1;
        while (document.getElementById("a"+k+","+(mod.index+4)))
	{
	        idx.push(mod.index + 4); //update drives
        	sidx.push(k);
	        flag.push(0);
        	len.push(255);
	        type.push("string");
        	i.push("6");
		k++;
	}

	if (idx.length>0)
	        new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, len, type, i, "Raid", true),1);
    }
}

function memoryloop()
{
    if (mod.type == "Memory")
    {
        var idx, sidx, flag, len, type, i;
        idx = [(mod.index + 1), (mod.index + 1), (mod.index + 1), (mod.index + 1), (mod.index + 1)];
        sidx = [1, 2, 3, 4, 5];
        flag = [0, 0, 0, 0, 0];
        len = [4, 4, 4, 4, 4];
        type = ["int", "int", "int", "int", "int"];
        i = ["0", "1", "2", "3", "4"];

        new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, len, type, i, "Memory", true),1);
    }
}

function favloop()
{
    if (mod.type == "PLC" && g_ndata !== null)
    {
		new Mutex (query_readwrite, new QueryData(browseindex + 4, 1, 2, buffer_size, "data", g_ndata, "GetFavVals", true),1);
    }
    else
    {
        g_ndata = null;
    }
}

function in_array(arr, elem)
{
    for (var i = 0; i < arr.length; i++)
    {
        if (arr[i] == elem)
            return true;
    }

    return false;
}

Array.prototype.deleteByElem = function (elem)
{
    for (var x = 0; x < this.length; ++x)
    {
        if (this[x] == elem)
        {
            this.deleteByKey(x);
            break;
        }
    }
};

Array.prototype.deleteByKey = function (key)
{
    for (var x = 0; x < this.length; ++x)
    {
        if (x >= key)
        {
            this[x] = this[x + 1];
        }

    }

    this.pop();
};

function runfunction(i, id, f)
{
    if (cancelreturn)
    {
        cancelreturn = false;
        return;
    }

    mod.run(i, id, f);
}

function getFunctionName(func)
{
	if ( typeof func == "function" || typeof func == "object" )
	var fName = (""+func).match(/function\s*([\w\$]*)\s*\(/);
	if ( fName !== null ) return fName[1];
}

function show_type(sData)
{
    if (!is_rebooting)
    {
        box_close();
        clear_info();

        if (sData.value != -1)
        {
            if (sData.value >= 0)
            {
                document.getElementById("current").innerHTML = "";
                mod = mod_list[sData.value];
            }

            if (!mod.hide)
                mod.show(true);
        }
        else
        {
            mod = "";
            show_overview();
        }

        if (sData.value != -2)
        {
            document.getElementById("m" + lastmenu + "i").className = "";
            document.getElementById("m" + lastmenu).className = "";

            document.getElementById("m" + sData.value + "i").className = "menuSelected";
            document.getElementById("m" + sData.value).className = "menuSelected";

            lastmenu = sData.value;
        }
    }
    if (sData.mutexIndex) Mutex.finish(sData.mutexIndex);
}

function count_types(type)
{
    var count = 0, i;

    for (i = 0; i < mod_count; i++)
    {
        if (mod_list[i].type == type)
            count++;
    }

    return count;
}

function get_object(type)
{
    for (var i = 0; i < mod_count; i++)
    {
        if (mod_list[i].type == type)
            return i;
    }

    return -1;
}

function favchange()
{
    var sel, ndata;

    sel = document.getElementById("favgroups").options[document.getElementById("favgroups").selectedIndex].value;

    if (sel == "none")
    {
        g_ndata = null;
        document.getElementById("cFavList").innerHTML = "";
        document.getElementById("function0").style.display = "";
    }
    else
    {
        ndata = [];
        favgroup = "PLC_FAVORITE_GROUP." + sel;
        ndata = prepare_data(16 + favgroup.length + 1, "int", 4, ndata);
        // cbData
        ndata = prepare_data(0, "int", 4, ndata);
        // hContinue
        ndata = prepare_data(buffer_size, "int", 4, ndata);
        // maxReadLen
        ndata = prepare_data(favgroup.length + 1, "int", 4, ndata);
        // cbPath
        ndata = prepare_data(favgroup, "string", 0, ndata);
        full_buffer = [];
        full_buffer_param = favgroup;

        new Mutex (query_readwrite, new QueryData(systemdsindex, 1, 2, buffer_size, "data", ndata, "GetFavList",false),0);
        document.getElementById("function0").style.display = "none";
    }
}

function getFav(id)
{
    var ndata = [], favgroup;

    favgroup = "PLC_FAVORITE_GROUP";
    ndata = prepare_data(16 + favgroup.length + 1, "int", 4, ndata);
    // cbData
    ndata = prepare_data(0, "int", 4, ndata);
    // hContinue
    ndata = prepare_data(buffer_size, "int", 4, ndata);
    // maxReadLen
    ndata = prepare_data(favgroup.length + 1, "int", 4, ndata);
    // cbPath
    ndata = prepare_data(favgroup, "string", 0, ndata);
    full_buffer = [];
    full_buffer_param = favgroup;

    new Mutex(query_readwrite, new QueryData(systemdsindex, 1, 2, buffer_size, "data", ndata, id, false),0);
}

function load_end()
{
	debugln("Running <strong>load_end()</strong>");
	
    document.getElementById("load_box").style.display = "none";
    document.getElementById("overlay").style.display = "none";

    if (fixDocumentSizeLoopID !== undefined)
        window.clearTimeout(fixDocumentSizeLoopID);
}

function load_start()
{
	debugln("Running <strong>load_start()</strong>");
	
    fixDocumentSizeLoop(500);
    document.getElementById("load_box").style.display = "";
    document.getElementById("overlay").style.display = "";
}

function fixOverlaySize()
{
    document.getElementById("overlay").style.height = (document.body.parentNode.scrollHeight) + "px";
    document.getElementById("overlay").style.width = (document.body.parentNode.scrollWidth) + "px";
}

function fixLoadBoxSize()
{
    document.getElementById("load_box").style.top = ((document.body.parentNode.offsetHeight / 2) - 30) + "px";
    document.getElementById("load_box").style.left = ((document.body.parentNode.offsetWidth / 2) - 125) + "px";
}

function fixDocumentSizeLoop(x)
{
    fixOverlaySize();
    fixLoadBoxSize();
    fixDocumentSizeLoopID = window.setTimeout("fixDocumentSizeLoop(" + x + ")", x);
}

// Image Preloader
function preloadImages(srcArr)
{
    imgArr = [];

    for (var i = 0; i < srcArr.length; i++)
    {
        if (document.images)
        {
            imgArr[i] = new Image();
            imgArr[i].src = srcArr[i];
        }
    }
}