function read_modes(data)
{
	mod_list = [];
	mod_count = 0;
	
	len = data.charCodeAt(0);
	
	debugln("found <b>"+len+"</b> objects");
	
	var lena;
	var ida = [];
	idx = [];
	sidx = [];
	flag = [];
	lena = [];
	type = [];
	
	
	for (i = 2; i < (len * 4); i = i + 4)
	{
	    var pos = data.charCodeAt(i) + data.charCodeAt(i + 1) * 256;
	    var id = data.charCodeAt(i + 2) + data.charCodeAt(i + 3) * 256;
	
	    idx.push(parseInt("0x8" + (pos < 16 ? "0": "") + pos.toString(16) + "0", 16));
	    sidx.push(2);
	    flag.push(0);
	    lena.push(64);
	    type.push(id);
	
	    idx.push(parseInt("0x8" + (pos < 16 ? "0": "") + pos.toString(16) + "0", 16));
	    sidx.push(3);
	    flag.push(0);
	    lena.push(128);
	    type.push(id);
	}
	
	new Mutex(query_readmultiple,new QueryData(idx, sidx, flag, lena, type, -1, "F020", false),0);
}

function encode_base64(input)
{
    var r = [];
    var chr1,
    chr2,
    chr3;
    var enc1,
    enc2,
    enc3,
    enc4;

    for (var i = 0; i < input.length;)
    {
        if (typeof(input) == "string")
        {
            chr1 = input.charCodeAt(i++);
            chr2 = input.charCodeAt(i++);
            chr3 = input.charCodeAt(i++);
        }
        else
        {
            chr1 = input[i++];
            chr2 = input[i++];
            chr3 = input[i++];
        }

        r[r.length] = b64s.charAt(chr1 >> 2);
        r[r.length] = b64s.charAt(((chr1 & 3) << 4) | (chr2 >> 4));
        r[r.length] = b64s.charAt(((chr2 & 15) << 2) | (chr3 >> 6));
        r[r.length] = b64s.charAt(chr3 & 63);
    }

    return r.join("");
}

function decode_base64(input)
{
    var output = "";
    var chr1,
    chr2,
    chr3;
    var enc1,
    enc2,
    enc3,
    enc4;
    var i = 0;


    // remove all characters that are not A-Z, a-z, 0-9, +, /, or =
    input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

    do
    {
        enc1 = b64s.indexOf(input.charAt(i++));
        enc2 = b64s.indexOf(input.charAt(i++));
        enc3 = b64s.indexOf(input.charAt(i++));
        enc4 = b64s.indexOf(input.charAt(i++));

        chr1 = (enc1 << 2) | (enc2 >> 4);
        chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
        chr3 = ((enc3 & 3) << 6) | enc4;

        output = output + String.fromCharCode(chr1);

        if (enc3 != 64)
        {
            output = output + String.fromCharCode(chr2);
        }

        if (enc4 != 64)
        {
            output = output + String.fromCharCode(chr3);
        }
    }
    while (i < input.length);

    return output;
}

function get_mdpoffset(index, subindex, flag)
{
    return index << 16 | subindex << 8 | flag;
}

function eval_data(k, i, text, len, type)
{
    var old_k = k;
    var c_val = "";
    var j = i * 4;
    var error = (text.charCodeAt(j + 3) << 24 | text.charCodeAt(j + 2) << 16 | text.charCodeAt(j + 1) << 8 | text.charCodeAt(j));

    if (error)
    {
        k = k + len;

        if (error != -2147220032)
            c_val = error;
    }
    else
    {
        if (type == "int")
        {
            if (len == 8)
                c_val = (text.charCodeAt(k + 7) << 56 | text.charCodeAt(k + 6) << 48 | text.charCodeAt(k + 5) << 40 | text.charCodeAt(k + 4) << 32 | text.charCodeAt(k + 3) << 24 | text.charCodeAt(k + 2) << 16 | text.charCodeAt(k + 1) << 8 | text.charCodeAt(k));
            else if (len == 4)
                c_val = (text.charCodeAt(k + 3) << 24 | text.charCodeAt(k + 2) << 16 | text.charCodeAt(k + 1) << 8 | text.charCodeAt(k));
            else
                c_val = (text.charCodeAt(k + 1) << 8 | text.charCodeAt(k));

            k = k + len;
        }
        else if (type == "bool")
        {
            c_val = text.charCodeAt(k);
            k = k + len;
        }
        else if (type == "nstring")
        {
            for (; (k - old_k) < (len); k++)
            {
                c_val += text.charAt(k);
            }
        }
        else
        {
            for (; (k - old_k) < (len); k++)
            {
                if (text.charCodeAt(k) != 0)
                    c_val += text.charAt(k);
                else
                {
                    k = len + old_k;
                    break;
                }
            }
        }
    }

    return new EvalData(k, c_val, error);
}

function parse_plc_vars(data, prefix)
{
    var version = (data.charCodeAt(5) << 24 | data.charCodeAt(4) << 16 | data.charCodeAt(3) << 8 | data.charCodeAt(2));

    switch (version)
    {

    case 1:
        break;

    case - 324532462:
        show_info("Error: PLC not running", "red");
        load_end();
        return;

    case - 324665083:
        load_end();
        return;

    default:
        show_info("Error: Invalid PLC-Object Version: " + version, "red");
        load_end();
        return;
    }

    var cbData = (data.charCodeAt(9) << 24 | data.charCodeAt(8) << 16 | data.charCodeAt(7) << 8 | data.charCodeAt(6));
    var hCont = (data.charCodeAt(13) << 24 | data.charCodeAt(12) << 16 | data.charCodeAt(11) << 8 | data.charCodeAt(10));
    var cbCurrent = (data.charCodeAt(17) << 24 | data.charCodeAt(16) << 16 | data.charCodeAt(15) << 8 | data.charCodeAt(14));
    var count = (data.charCodeAt(21) << 24 | data.charCodeAt(20) << 16 | data.charCodeAt(19) << 8 | data.charCodeAt(18));
    var next = (data.charCodeAt(25) << 24 | data.charCodeAt(24) << 16 | data.charCodeAt(23) << 8 | data.charCodeAt(22)) + 2;

    if (cbData > buffer_size)
    {
        var j = full_buffer.length;

        for (var k = (j == 0 ? 0: 26); k < (cbCurrent + 2); k++)
        {
            full_buffer[j++] = data.charCodeAt(k);
        }
    }
    else
    {
        var j = 0;

        for (var k = 0; k < (cbData + 2); k++)
        {
            full_buffer[j++] = data.charCodeAt(k);
        }

        data = full_buffer;
    }

    if (hCont == 0 && cbData > buffer_size)
    {
        data = full_buffer;
    }
    else if (hCont != 0 && cbData > buffer_size)
    {
        var ndata = [];
        ndata = prepare_data(0, "int", 4, ndata);
        ndata = prepare_data(full_buffer_param[0] + 24, "int", 4, ndata);
        ndata = prepare_data(hCont, "int", 4, ndata);
        ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
        ndata = prepare_data(full_buffer_param[1], "int", 4, ndata);
        ndata = prepare_data(24, "int", 4, ndata);
        ndata = append_data(full_buffer_param[2], ndata);

        new Mutex (query_readwrite, new QueryData(browseindex + 4, 1, 2, buffer_size, "data", ndata, "GetPlcValues", false), 0);
        return;
    }

    debugln("Received PlcVar data: " + count + " variables");
    full_buffer = [];
    currenttemp = [];

    var currentVar = 0;

    for (var i = 0; i < count; i++)
    {
        currentVar = next;

        next = (data[currentVar + 3] << 24 | data[currentVar + 2] << 16 | data[currentVar + 1] << 8 | data[currentVar]) + 2;
        size = (data[currentVar + 7] << 24 | data[currentVar + 6] << 16 | data[currentVar + 5] << 8 | data[currentVar + 4]);

        try
        {
            if (mod.type == "PLC")
            {
                var dir = "";
                var type = document.getElementById(prefix + "plctype" + i).innerHTML;

                for (var j = 0; j < size; j++)
                {
                    dir += String.fromCharCode(data[currentVar + 8 + j]);
                }

                dir = get_plc_val(type, size, dir);
                document.getElementById(prefix + "plcvar" + i).innerHTML = dir;
            }
        }
        catch (e) {}

    }

}

function parse_plc(data, mode)
{
    currenttemp = [];

    var version = (data.charCodeAt(5) << 24 | data.charCodeAt(4) << 16 | data.charCodeAt(3) << 8 | data.charCodeAt(2));

    var foldertable = "<table class=\"menutable\" width=\"100%\">";

    if (folderparent != "")
    {
        var pos1,
        pos2,
        pos;
        pos1 = folderparent.lastIndexOf("[")
               pos2 = folderparent.lastIndexOf(".")

                      if (pos1 > pos2)
                          pos = pos1;
        else
            pos = pos2;

        if (pos == -1)
            parent2 = "";
        else
        {
            parent2 = folderparent.substring(0, pos);
        }

        if (parent2.charAt(parent2.length - 1) == ".")
        {
            parent2 = parent2.substring(0, parent2.length - 1);
        }

        foldertable += "<tr><td width=\"19\"><a href=\"javascript:folderchange('" + parent2 + "','','PLC','')\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></a></td><td align=\"left\" valign=\"middle\"><a href=\"javascript:folderchange('" + parent2 + "','','PLC','')\">..</a></td></tr>";
    }

    switch (version)
    {

    case 1:
        break;

    case -324532462:
        show_info("Error: PLC not running", "red");
        load_end();
        return;

    case -324665083:
        if (folderparent != "")
            document.getElementById("varfolders").innerHTML = foldertable + "</table>";
        else
            document.getElementById("currentfoldercontainer").innerHTML = "empty";

        load_end();

        return;

    default:
        show_info("Error: Invalid PLC-Object Version: " + version, "red");

        load_end();

        return;
    }

    var cbData = (data.charCodeAt(9) << 24 | data.charCodeAt(8) << 16 | data.charCodeAt(7) << 8 | data.charCodeAt(6));
    var hCont = (data.charCodeAt(13) << 24 | data.charCodeAt(12) << 16 | data.charCodeAt(11) << 8 | data.charCodeAt(10));
    var cbCurrent = (data.charCodeAt(17) << 24 | data.charCodeAt(16) << 16 | data.charCodeAt(15) << 8 | data.charCodeAt(14));
    var cbItems = (data.charCodeAt(21) << 24 | data.charCodeAt(20) << 16 | data.charCodeAt(19) << 8 | data.charCodeAt(18));
    var nextDir = (data.charCodeAt(25) << 24 | data.charCodeAt(24) << 16 | data.charCodeAt(23) << 8 | data.charCodeAt(22)) + 2;

    if (cbData > buffer_size)
    {
        var j = full_buffer.length;

        for (var k = (j == 0 ? 0: 26); k < (cbCurrent + 2); k++)
        {
            full_buffer[j++] = data.charCodeAt(k);
        }
    }
    else
    {
        var j = 0;

        for (var k = 0; k < (cbData + 2); k++)
        {
            full_buffer[j++] = data.charCodeAt(k);
        }

        data = full_buffer;
    }

    if (hCont == 0 && cbData > buffer_size)
    {
        data = full_buffer;
    }
    else if (hCont != 0 && cbData > buffer_size)
    {
        debugln("Buffer full, next query...");
        var ndata = [];
        ndata = prepare_data(0, "int", 4, ndata);
        // version
        ndata = prepare_data(20 + full_buffer_param.length + 1, "int", 4, ndata);
        // cbData
        ndata = prepare_data(hCont, "int", 4, ndata);
        // hContinue
        ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
        // maxReadLen
        ndata = prepare_data(full_buffer_param.length + 1, "int", 4, ndata);
        // cbPath
        ndata = prepare_data(full_buffer_param, "string", 0, ndata);
        // szPath
        new Mutex (query_readwrite, new QueryData(browseindex, 1, 2, buffer_size, "data", ndata, "Path", false),0);
        return;
    }

    debugln("All data received");
    full_buffer_param = "";
    full_buffer = [];
    var currentDir;

    for (var i = 0; i < cbItems; i++)
    {
        currentDir = nextDir;
        nextDir = (data[currentDir + 3] << 24 | data[currentDir + 2] << 16 | data[currentDir + 1] << 8 | data[currentDir]) + 2;
        var size = (data[currentDir + 7] << 24 | data[currentDir + 6] << 16 | data[currentDir + 5] << 8 | data[currentDir + 4]);
        var cbadstype = (data[currentDir + 11] << 24 | data[currentDir + 10] << 16 | data[currentDir + 9] << 8 | data[currentDir + 8]);
        var type = (data[currentDir + 15] << 24 | data[currentDir + 14] << 16 | data[currentDir + 13] << 8 | data[currentDir + 12]);
        var vsize = (data[currentDir + 19] << 24 | data[currentDir + 18] << 16 | data[currentDir + 17] << 8 | data[currentDir + 16]);
        var hasS = data[currentDir + 20];

        var dir = "";
        var adstype = "";
        var j;

        for (j = 0; j < size; j++)
        {
            dir += String.fromCharCode(data[currentDir + 21 + j]);
        }

        for (; j < (size + cbadstype); j++)
        {
            adstype += String.fromCharCode(data[currentDir + 21 + j]);
        }

        currenttemp.push(dir);
        currenttemp.push(adstype);
        currenttemp.push(vsize);
        currenttemp.push(hasS);
    }

    var folder = "";

    if (currenttemp.length == 0)
    {
        var tmp = "";

        try
        {
            tmp = document.getElementById("selectedvar").value
              }
        catch (e) {}

        if (tmp != "")
        {
            var ndata = [];
            ndata = prepare_data(tmp.length + 1, "int", 4, ndata);
            ndata = prepare_data(tmp, "string", 0, ndata);
            new Mutex (query_readwrite, new QueryData(browseindex + 2, 1, 2, buffer_size, "data", ndata, "GetPlcVars", false),0);
            foldertable += "<tr><td colspan=\"2\"><span id=\"plcvalue\">Loading...</span></td></tr>";
        }
    }

    var tmp = [];
    var tmpcbdata = 0;
    var j = 0;

    for (i = 0; i < currenttemp.length; i++)
    {
        folder = currenttemp[i];
        type = currenttemp[++i];
        vsize = currenttemp[++i];
        hasS = currenttemp[++i];

        var opt = "";

        if (type != 0 && hasS == false)
        {
            opt += type;

            if (vsize != 0)
                opt += " (" + vsize + ")";
        }

        pos = folder.lastIndexOf(".");

        if (pos != -1 && pos != 0)
        {
            folder2 = folder.substring(pos + 1, folder.length);
        }
        else
            folder2 = folder;

        var linkS = "";

        if (hasS)
        {
            linkS = "<a href=\"javascript:folderchange('" + folder2 + "','" + double_slashes(folderparent) + "','PLC', '')\">";
            value = "";
        }
        else
        {
            var name = folderparent + (folderparent != "" ? ".": "") + folder2;

            if (type != "")
            {
                linkS = "<a id=plclink" + j + " style=\"text-decoration: none\" href=\"javascript:plcvar_open('" + j + "','')\">";
            }

            if (type == "" || type.substring(0, 7) != "ARRAY [")
            {
                var folderfull = ((folderparent.length > 0 ? folderparent + ".": "") + folder);
                tmp.push(folderfull);
                tmpcbdata += folderfull.length + 8;
            }

            value = "<tr><td width=\"19\"></td><td class=\"smallfont\"><span id=\"plcvar" + j + "\"></span>&nbsp;";

            if (type != "")
            {
                value += "<span id=\"plcname" + j + "\" style=\"display:none\">" + name + "</span>";
                value += "<span id=\"plctype" + j + "\" style=\"display:none\">" + type + "</span>";
                value += "<span id=\"plcsize" + j + "\" style=\"display:none\">" + vsize + "</span>";
            }

            value += "</td></tr>";
            j++;
        }

        foldertable += "<tr><td width=\"19\">" + linkS + "<img alt=\"\" border=\"0\" src=\"images/" + (hasS ? "OZ": "ADDFAV") + ".gif\"></a></td><td valign=\"middle\">" + linkS + opt + " " + folder + "</a></td></tr>" + value;
    }

    if (tmp != null && tmp.length > 0)
    {
        var ndata = [];
        ndata = prepare_data(0, "int", 4, ndata);
        ndata = prepare_data(tmpcbdata + 24, "int", 4, ndata);
        ndata = prepare_data(0, "int", 4, ndata);
        ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
        ndata = prepare_data(tmp.length, "int", 4, ndata);
        ndata = prepare_data(24, "int", 4, ndata);
        var j = 24;
        var ndata2 = [];

        for (var i = 0; i < tmp.length; i++)
        {
            j += tmp[i].length + 8;

            ndata2 = prepare_data(j, "int", 4, ndata2);
            ndata2 = prepare_data(tmp[i].length, "int", 4, ndata2);
            ndata2 = prepare_data(tmp[i], "string", 0, ndata2);
        }

        full_buffer_param = [];
        full_buffer_param[0] = tmpcbdata;
        full_buffer_param[1] = tmp.length;
        full_buffer_param[2] = ndata2;

        ndata = append_data(ndata2, ndata);

        new Mutex (query_readwrite, new QueryData(browseindex + 4, 1, 2, buffer_size, "data", ndata, "GetPlcVars", false),0);
        ndata = null;
        ndata2 = null;
    }
    else
        load_end();

    foldertable += "</table>";

    document.getElementById("varfolders").innerHTML = foldertable;
}

function plcvar_open(j, fav)
{
    var a = getPosition(document.getElementById((fav?"f":"")+"plcvar" + j));
    var varname = document.getElementById((fav?"f":"")+"plcname" + j).innerHTML;
    var type = document.getElementById((fav?"f":"")+"plctype" + j).innerHTML;
    var size = document.getElementById((fav?"f":"")+"plcsize" + j).innerHTML;
    var value = document.getElementById((fav?"f":"")+"plcvar" + j).innerHTML;
    var id = (fav?"faveditbox":"plceditbox");

    var text = "<table><tr><td><b>Type</b></td><td><span id=\"plcvar_type\">" + type + "</span></td><tr><td><b>Value</b></td>";

    text += "<td><table><tr><td><img onclick=\"edit('" + j + "','"+value+"','" + id + "')\" src=\"IMAGES/edit.gif\" \></td><td><span id=\"" + id + "\"><span style=\"width:100%;\" onclick=\"edit('" + j + "',this.innerHTML,'" + id + "')\">" + value + "</span></span></td></tr></table></td>";

	 if (!fav)
	 {
	    text += "</tr></table><br><br><b>Add to favorites</b><br>";
	
	    // list groups
	    text += "<table><tbody id=\"favList\">";
	
	    var plci = get_object("PLC");
	
	    if (plci == -1)
	    {
	        return;
	    }
	
	    for (var i = 0; mod_list[plci].favorites != null && i < mod_list[plci].favorites.length; i++)
	    {
	        text += "<tr><td><a href=\"javascript:plcfav_addvar('" + mod_list[plci].favorites[i] + "','" + varname + "','" + type + "','" + size + "')\">" + mod_list[plci].favorites[i] + "</a></td></tr>";
	    }
	
	    text += "</tbody></table><span id=\"plcvar_name\" style=\"display: none\">" + varname + "</span>";
	
	    text += "<p><table class=\"datatable\" width=\"90%\" cellpadding=\"1\" cellspacing=\"1\" align=\"center\">";
	    text += "<tr><td class=\"titletable\" width=\"40%\" valign=\"top\"><b>New Group</b></td><td class=\"normaltable\" width=\"60%\">" + format_add("", "asizename", false) + "</td>";
	    text += "<td class=\"normaltable\"><input class=\"button\" type=\"submit\" onclick=\"javascript:plcfav_addvar('', '" + varname + "', '" + type + "','" + size + "')\" value=\"Add\"></td></tr></table></p>";
    }
    currenttemp.length = 0;

    var pos = getPosition(document.getElementById((fav?"f":"")+"plclink" + j));
    box_open(varname, text, pos.x - (fav?50:150), pos.y - (fav?50:150), 300);
}

function plcfav_addvar(group, name, type, size)
{
    var glob = "0"

    if (name.charAt(0) == '.')
    {
        cname = name.substring(1, name.length);
        glob = "1";
    }
    else
        cname = name;

    if (name.length == 0 || type.length == 0)
        return;

    if (group == "")
    {
        try
        {
            group = document.getElementById("asizename").value;
        }
        catch (e)
        {
            return;
        }

        if (group == "")
            return;
    }

    cname = cname.replace(/\./g, "-");
    
    var rname = "PLC_FAVORITE_GROUP." + group + "." + cname;
    var nlen = rname.length;
    var rvalue = glob + "-" + type + "-" + size;
    var vlen = rvalue.length;

    var len = type.length;

    var data = [];
    data = prepare_data(nlen + vlen + 12, "int", 4, data);
    data = prepare_data(nlen, "int", 4, data);
    data = prepare_data(vlen, "int", 4, data);
    data = prepare_data(rname, "string", 0, data);
    data = prepare_data(rvalue, "string", 0, data);

    var qData = new QueryData(systemdsindex+1, 1, 0, 0, "data", data, "ADDFAVGROUP", false);
    new Mutex(query_write,qData,0);

    currenttemp = group;
    box_close();
}

function remove_cp(name)
{
    for (var i = 0; i < mod_list.length; i++)
    {
        if (mod_list[i].name == name && mod_list[i].type == "Customer Pages")
        {
            mod_list.splice(i, 1);
            mod_count--;
            draw_menu();
            lastmenu = -1;

            if (document.is_ie)
                document.getElementById('ajaxnav').setAttribute('src', 'position.asp?hash=-1');
            else new Mutex(show_type,new ShowData(-1),0);

            break;
        }
    }
}

function parse_ds(data, mode)
{
    debugln("Parsing DataStore: <b>" + mode + "</b>");


    if (mode == "CP" && get_object("Customer Pages") == -1)
    {
        mod_list[mod_count++] = new Module("Customer Pages", "Customer Pages", "", 513);
    }

    if (mode == "FAVG")
    {
        var plci = get_object("PLC");

        if (plci != -1)
        {
            mod_list[plci].favorites = [];
        }
        else
            return;
    }

    currenttemp = [];

    var version = (data.charCodeAt(5) << 24 | data.charCodeAt(4) << 16 | data.charCodeAt(3) << 8 | data.charCodeAt(2));

    if (version == -324665083)
        // list empty
    {
        debugln("Selected datastore path is <b>empty</b>");

        if (mode != "CP" && mode != "FAVG" && mod.type != "PLC")
            document.getElementById("currentfoldercontainer").innerHTML = "empty";
        else
            draw_menu();

        return;
    }
    else if (version != 1)
    {
        if (mode != "FAVG")
        {
            show_info("Error: Invalid DS-Object Version: " + version, "red");
        }

        load_end();
        return;
    }

    var cbData = (data.charCodeAt(9) << 24 | data.charCodeAt(8) << 16 | data.charCodeAt(7) << 8 | data.charCodeAt(6));
    var hCont = (data.charCodeAt(13) << 24 | data.charCodeAt(12) << 16 | data.charCodeAt(11) << 8 | data.charCodeAt(10));
    var cbVars = (data.charCodeAt(17) << 24 | data.charCodeAt(16) << 16 | data.charCodeAt(15) << 8 | data.charCodeAt(14));
    var cbCurrent = (data.charCodeAt(21) << 24 | data.charCodeAt(20) << 16 | data.charCodeAt(19) << 8 | data.charCodeAt(18))
                    var nextVar = (data.charCodeAt(25) << 24 | data.charCodeAt(24) << 16 | data.charCodeAt(23) << 8 | data.charCodeAt(22)) + 2;

    if (cbData > buffer_size)
    {
        var j = full_buffer.length;

        for (var k = (j == 0 ? 0: 26); k < (cbCurrent + 2); k++)
        {
            full_buffer[j++] = data.charCodeAt(k);
        }
    }
    else
    {
        var j = 0;

        for (var k = 0; k < (cbData + 2); k++)
        {
            full_buffer[j++] = data.charCodeAt(k);
        }

        data = full_buffer;
    }

    if (hCont == 0 && cbData > buffer_size)
    {
        data = full_buffer;
    }
    else if (hCont != 0 && cbData > buffer_size)
    {
        var ndata = [];
        ndata = prepare_data(16 + full_buffer_param.length + 1, "int", 4, ndata);
        // cbData
        ndata = prepare_data(hCont, "int", 4, ndata);
        // hContinue
        ndata = prepare_data(buffer_size - 2, "int", 4, ndata);
        // maxReadLen
        ndata = prepare_data(full_buffer_param.length + 1, "int", 4, ndata);
        // cbPath
        ndata = prepare_data(full_buffer_param, "string", 0, ndata);
        new Mutex (query_readwrite, new QueryData(systemdsindex, 1, 2, buffer_size, "data", ndata, (cp ? "CustomerPagesGet": "DataStoreGet"), false),0);
        return;
    }

    var param = full_buffer_param;
    full_buffer_param = "";
    full_buffer = [];

    var currentVar;

    for (var i = 0; i < cbVars; i++)
    {
        currentVar = nextVar;
        var sizeN = (data[currentVar + 3] << 24 | data[currentVar + 2] << 16 | data[currentVar + 1] << 8 | data[currentVar]);
        var sizeV = (data[currentVar + 7] << 24 | data[currentVar + 6] << 16 | data[currentVar + 5] << 8 | data[currentVar + 4]);
        var sub = data[currentVar + 8];
        var nextVar = (data[currentVar + 12] << 24 | data[currentVar + 11] << 16 | data[currentVar + 10] << 8 | data[currentVar + 9]) + 2;

        var dir = "";
        var value = "";

        for (var j = 0; j < sizeN; j++)
        {
            dir += String.fromCharCode(data[currentVar + 13 + j]);
        }

        for (var j = 0; j < sizeV; j++)
        {
            value += String.fromCharCode(data[currentVar + j + sizeN + 13]);
        }

        currenttemp.push(dir);
        currenttemp.push(value);
        currenttemp.push(sub);
    }

    if (mode == "CP")
    {
        while (currenttemp.length > 0)
        {
            sub = currenttemp.pop();
            value = currenttemp.pop();
            name = currenttemp.pop();
            mod_list[mod_count++] = new Module(name, "Customer Pages", 0, 513);
            mod_list[mod_count - 1].url = value;
        }

        window.setTimeout(function()
        {
            getFav("GetFav")
        }, 200);
        return;
    }
    else if (mode == "DS")
    {
        var vartable = "<table class=\"menutable\">";

        if (folderparent != "")
        {
            var pos;
            pos = folderparent.lastIndexOf(".");

            if (pos == -1)
            {
                parent2 = "";
            }
            else
            {
                parent2 = folderparent.substring(0, pos);
            }

            if (parent2.charAt(parent2.length - 1) == ".")
            {
                parent2 = parent2.substring(0, parent2.length - 1);
            }

            vartable += "<tr><td width=\"19\"><a href=\"javascript:folderchange('" + parent2 + "','',true, 'DS')\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></a></td><td colspan=\"2\" align=\"left\" valign=\"middle\"><a href=\"javascript:folderchange('" + parent2 + "','',true,'DataStoreGet')\">..</a></td></tr>";
        }

        var var1 = "";

        if (currenttemp.length == 0)
            vartable += "<tr><td colspan=\"2\">no subvars</td></tr>";

        while (currenttemp.length > 0)
        {
            sub = currenttemp.pop();
            value = currenttemp.pop();
            name = currenttemp.pop();

            if (sub)
                nameN = "<a href=\"javascript:folderchange('" + name + "','" + folderparent + "','DS','DataStoreGet')\">" + name + "</a>";
            else
                nameN = name;

            if (folderparent != "")
                name = folderparent + "." + name;

            var del = "";

            vartable += "<tr><td width=\"19\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></td><td width=\"50%\" valign=\"middle\">" + nameN + "</td><td width=\"40%\">" + value + "</td><td width=\"19\"><a href=\"javascript:delete_ds('" + name + "'," + mod.functions[2].index + ", " + mod.functions[2].refresh + ", '')\"><img border=\"0\" src=\"images/delete.gif\"></a></td></tr>";
        }

        vartable += "</table>";
        document.getElementById("varfolders").innerHTML = vartable;
    }
    else if (mode == "FAV")
    {
        text = "<table width=\"100%\">";
        var tmp = [];
        var tmpcbdata = 0;
        var i = 0;
        var groupname = param;

        while (currenttemp.length > 0)
        {
            var sub = currenttemp.pop();
            var value = currenttemp.pop();
            var name = currenttemp.pop();
            var displayname = name.replace("-", ".");

            vArr = value.split("-");

            if (vArr.length == 3)
            {
                if (vArr[0] == "1")
                    displayname = "." + displayname;

                tmp.push(displayname);

                tmpcbdata += displayname.length + 8;

                text += "<tr><td width=\"35%\" class=\"smallfont\"><a id=fplclink" + i + " href=\"javascript:plcvar_open('" + i + "','1')\">" + displayname + "</a><span id=\"fplcname"+i+"\" style=\"display:none;\">"+displayname+"</span></td><td width=\"25%\" class=\"smallfont\"><span id=\"fplctype" + i + "\">" + vArr[1] + "</span> (<span id=\"fplcsize" + i + "\">" + vArr[2] + "</span>)</td><td class=\"smallfont\"><span id=\"fplcvar" + i + "\" width=\"35%\"></span>&nbsp;</td><td><a href=\"javascript:delete_ds('" + groupname + "." + name + "'," + (systemdsindex + 2) + "," + true + ",'')\"><img alt=\"Remove\" border=\"0\" src=\"images/delete.gif\"></a></td></tr>";

                i++;
            }
        }

        text += "</table><br><a href=\"javascript:delete_fav('" + groupname.substring("PLC_FAVORITE_GROUP".length + 1) + "')\">delete group</a>";

        var ndata = [];
        ndata = prepare_data(0, "int", 4, ndata);
        ndata = prepare_data(tmpcbdata + 24, "int", 4, ndata);
        ndata = prepare_data(0, "int", 4, ndata);
        ndata = prepare_data(buffer_size, "int", 4, ndata);
        ndata = prepare_data(tmp.length, "int", 4, ndata);
        ndata = prepare_data(24, "int", 4, ndata);
        var j = 24;
        var ndata2 = [];

        for (var i = 0; i < tmp.length; i++)
        {
            j += tmp[i].length + 8;
            ndata2 = prepare_data(j, "int", 4, ndata2);
            ndata2 = prepare_data(tmp[i].length, "int", 4, ndata2);
            ndata2 = prepare_data(tmp[i], "string", 0, ndata2);
        }

        currenttemp = ndata2;
        ndata = append_data(ndata2, ndata);

        if (g_interval)
            window.clearInterval(g_interval);

        g_ndata = ndata;

        g_interval = window.setInterval(favloop, 1000);

        document.getElementById("cFavList").innerHTML = text;
    }
    else if (mode == "FAVG")
    {
        while (currenttemp.length > 0)
        {
            sub = currenttemp.pop();
            value = currenttemp.pop();
            name = currenttemp.pop();

            mod_list[plci].favorites.push(name);
        }
    }
}

function delete_fav(name)
{
    delete_ds("PLC_FAVORITE_GROUP." + name, (systemdsindex + 2), false, "REMOVEFAVGROUP");
    currenttemp = name;

    if (g_interval)
        window.clearInterval(g_interval);
}

function delete_ds(name, idx, refresh, id)
{
    if ((name.substring("CP.", 3) == "CP.") && idx == (systemdsindex + 2))
        remove_cp(name.substring(3, name.length));

    var data = [];
    data = prepare_data(name.length, "int", 4, data);
    data = prepare_data(name, "string", 0, data);

    var qData = new QueryData(idx, 1, 2, 0, "data", data, id, false);
    qData.refresh = refresh;
    new Mutex(query_write,qData,0);
}

function parse_dirs(data)
{
    currenttemp = [];

    var version = (data.charCodeAt(5) << 24 | data.charCodeAt(4) << 16 | data.charCodeAt(3) << 8 | data.charCodeAt(2));

    if (version != 0)
    {
        show_info("Error: Invalid File-Object Version: " + version, "red");
        load_end();
        return;
    }

    var cbData = (data.charCodeAt(9) << 24 | data.charCodeAt(8) << 16 | data.charCodeAt(7) << 8 | data.charCodeAt(6));
    var cbDirs = (data.charCodeAt(13) << 24 | data.charCodeAt(12) << 16 | data.charCodeAt(11) << 8 | data.charCodeAt(10));

    var cbFiles = (data.charCodeAt(17) << 24 | data.charCodeAt(16) << 16 | data.charCodeAt(15) << 8 | data.charCodeAt(14));
    var nextDir = (data.charCodeAt(21) << 24 | data.charCodeAt(20) << 16 | data.charCodeAt(19) << 8 | data.charCodeAt(18)) + 2;

    var currentDir;

    for (var i = 0; i < cbDirs; i++)
    {
        currentDir = nextDir;
        nextDir = (data.charCodeAt(currentDir + 3) << 24 | data.charCodeAt(currentDir + 2) << 16 | data.charCodeAt(currentDir + 1) << 8 | data.charCodeAt(currentDir)) + 2;
        var size = (data.charCodeAt(currentDir + 7) << 24 | data.charCodeAt(currentDir + 6) << 16 | data.charCodeAt(currentDir + 5) << 8 | data.charCodeAt(currentDir + 4));

        var dir = "";

        for (var j = 0; j < size; j++)
        {
            dir += data.charAt(currentDir + 8 + j);
        }

        currenttemp.push(dir);

    }

    var foldertable = "<table class=\"menutable\">";

    if (folderparent != "" && folderparent != "\\" && !(folderparent.charAt(1) == ":" && folderparent.length <= 3))
    {
        if (folderparent.charAt(0) == "\\")
            folderparent = folderparent.substring(1, folderparent.length);

        var parent2 = "";

        var pos = folderparent.lastIndexOf("\\")
                  if (pos == folderparent.length - 1)
        {
            folderparent = folderparent.substring(0, pos);
            pos = folderparent.lastIndexOf("\\")
              }

              if (pos == -1)
                  parent2 = "";
        else
        {
            parent2 = folderparent.substring(0, pos);
        }

        parent2 = double_slashes(parent2);
        foldertable += "<tr><td width=\"19\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></td><td width=\"100%\" align=\"left\" valign=\"middle\"><a href=\"javascript:folderchange('" + parent2 + "','','DIR', '')\">..</a></td></tr>";
    }

    var folder = "";

    if (currenttemp.length == 0)
        foldertable += "<tr><td colspan=\"2\">no subfolders</td></tr>";

    for (i = 0; i < currenttemp.length; i++)
    {
        folder = currenttemp[i];
        foldertable += "<tr><td width=\"19\"><img alt=\"\" border=\"0\" src=\"images/OZ.gif\"></td><td valign=\"middle\"><a href=\"javascript:folderchange('" + folder + "','" + double_slashes(folderparent) + "','DIR','')\">" + folder + "</a></td></tr>";
    }

    foldertable += "</table>";
    document.getElementById("subfolders").innerHTML = foldertable;
}

function to_byte(v)
{
    return parseInt(v, 10) & 255;
}

function append_data(data, ndata)
{
    var j = ndata.length;

    for (var k = 0; k < data.length; k++)
    {
        ndata[j++] = data[k];
    }

    return ndata;
}

function prepare_data(data, type, len, ndata)
{
    var j = ndata.length;

    if (type == "string")
    {
        var k;

        for (k = 0; k < data.length; k++)
        {
            ndata[j++] = data.charCodeAt(k);
        }

        for (; k < len; k++)
        {
            ndata[j++] = 0;
        }

    }
    else if (type == "bool")
    {
        ndata[j++] = data;
    }
    else if (type == "data")
    {
        for (var k = 0; k < data.length; k++)
        {
            ndata[j++] = data[k];
        }
    }
    else if (type == "netid")
    {
        data = data.split(".");

        if (data.length != 6)
            data = [0, 0, 0, 0, 0, 0];

        for (var k = 0; k < data.length; k++)
        {
            ndata[j++] = data[k];
        }
    }
    else
    {
        if (len == 1)
        {
            ndata[j++] = to_byte(parseInt((data >> (0)), 10));
        }
        else if (len == 2)
        {
            data = parseInt(data);
            ndata[j++] = to_byte(parseInt((data >> (0)), 10));
            ndata[j++] = to_byte(parseInt((data >> (8)), 10));
        }
        else
        {
            data = parseInt(data);

            if (data == "NaN")
                data = 0;

            ndata[j++] = to_byte(parseInt((data >> (0)), 10));

            ndata[j++] = to_byte(parseInt((data >> (8)), 10));

            ndata[j++] = to_byte(parseInt((data >> (16)), 10));

            ndata[j++] = to_byte(parseInt((data >> (24)), 10));
						
        }
    }

    return ndata;
}

function double_slashes(string)
{
    var newstring = "";

    for (var i = 0; i < string.length; i++)
    {
        newstring += string.charAt(i);

        if (string.charAt(i) == "\\" && string.charAt(i + 1) != "\\")
        {
            newstring += "\\";
        }
    }

    return newstring;
}

function is_true(value)
{
    if (value == null || value == "false" || value == "False" || value == "" || value == 0 || value == "0" || value == "No" || value == "False" || value == "Disabled")
    {
        return false;
    }

    return true;
}


function to_plc_val(type, size, data)
{
    var retval = "";

    if (type == "WORD" || type == "DWORD" || type == "UDINT" || type == "DINT" || type == "INT16" || type == "UINT16" || type == "BYTE" || type == "TIME" || type == "DATE" || type == "BOOL")
    {
        if (size == 1)
        {
            retval = String.fromCharCode(is_true(data)?1:0);
        }
        else
        {
            if (type == "DATE")
            {
            	 var tmp = data.split(".");
                var date = new Date();
                date.setDate(tmp[0]);
                date.setMonth(tmp[1]+1);
                date.setFullYear(tmp[2]);
                retval = date.valueOf() * 1000;
            }
            else
            {
            	data = parseFloat(data);
					
					var neg = false;
					if ((type == "WORD" || type == "DWORD" || type == "INT16") && size > 1)
					{
						 if (data < 0)
						 {
						 	 neg = true;
						 	 data = data + Math.pow(size, 16);
						 }					        
					}
					
					for (var i = 0; i < size; i++)
					{
					    retval += String.fromCharCode(data >> (8 * i));
					}					
				}
        }
    }
    else if (type == "LREAL" || type == "REAL")
    {
        return Real2Bin(data, type);
    }
    else if (type.startsWith("ARRAY "))
    {
        return false;
    }
    else if (type.match("^STRING\(\d\)") != -1)
    {
        return data;
    }
    else
        retval = false;

    return retval;
}

function get_plc_val(type, size, data)
{
    var retval = "";

    if (type == "WORD" || type == "DWORD" || type == "UDINT" || type == "DINT" || type == "INT16" || type == "UINT16" || type == "BYTE" || type == "TIME" || type == "DATE" || type == "BOOL")
    {
        if (size == 1)
        {
            if (data.charCodeAt(0))
                retval = "True";
            else
                retval = "False";
        }
        else
        {
            var neg = false;

            for (var i = 0; i < size; i++)
            {
                retval |= data.charCodeAt(i) << (8 * i);
            }

            if ((type == "WORD" || type == "DWORD" || type == "INT16") && size > 1)
            {
                if (retval > (Math.pow(size, 16) / 2))
                    retval = retval - Math.pow(size, 16);
            }
            else if (type == "DATE")
                // timestamp
            {
                var date = new Date(parseInt(retval) * 1000);
                var dd = date.getDate();
                var mm = date.getMonth() + 1;
                var yy = date.getFullYear();
                retval = (dd < 10 ? "0": "") + dd + "." + (mm < 10 ? "0": "") + mm + "." + yy;
            }
        }
    }
    else if (type == "LREAL" || type == "REAL")
    {
        retval = toReal(data, type);
    }
    else if (type.startsWith("ARRAY "))
    {
        return "";
    }
    else if (type.match("^STRING\(\d\)") != -1)
    {
        for (var i = 0; i < size; i++)
        {
            if (data.charCodeAt(i) != 0)
                retval += data.charAt(i);
        }
    }
    else
        retval = "";

    return retval;
}

function i2b(value)
{
    var buf = "";
    var buf2 = "";
    var quotient = value;
    var i = 0;

    do
    {
        buf += (Math.floor(quotient % 2) == 1 ? "1": "0");
        quotient /= 2;
        i++;
    }
    while (i < 8);

    buf = buf.split("").reverse().join("");

    return buf;
}


function Real2Bin(value, type)
{
        //sign bit
        var sign;
        if (value>0)
                sign = 0;
        else sign = 1;
        
	    switch (type)
	    {
	
	    case "LREAL":
	        exp = 11;
	        man = 52;
	        bias = 1023;
	        break;
	
	    default:
	        exp = 8;
	        man = 23;
	        bias = 127;
	    }
        
        
        var n = 0;
        var power;
        var sign2;        
        if (value<2 && value>-2)
                sign2=-1;
        else sign2 = 1;       
        for (power=0; n <= 1 || n>2; ++power)
        {
				n = Math.pow(-1, sign) * value / (Math.pow(2, sign2*power));
        }
        power--;
  
        var exponent = bias+power;
        exponent = exponent.toString(2);
        
        for (var i = exponent.length; i < exp; i++)
        {
                exponent = "0" + exponent;
        }
        
        var n2=0;
        var temp=0;
        var fraction="";
        n = n - 1;
        for (var i = 1; i<(man+1); i++)
        {
                temp = n2 + 1/Math.pow(2,i);
                if (temp<=n)
                {
                        n2 = temp;
                        fraction += "1";
                }
                else fraction += "0";
        }
        
        var tmp = sign+exponent+fraction;
                
        var retval = "";
        
        for (var i = 0; i<tmp.length; i+=8)
        {
        		retval+=String.fromCharCode(b2i(tmp.substr(i,8)));
		  }
		  retval = retval.split("").reverse().join("");
		  
		  return retval;
}


function toReal(string, type)
{
    switch (type)
    {

    case "LREAL":
        len = 8;
        break;

    default:
        len = 4;
        break;
    }

    var bin = "";

    for (var i = len - 1; i >= 0; i--)
    {
        bin += i2b(string.charCodeAt(i));
    }

    return Bin2Float(bin, type);
}

function Bin2Float(binary, type)
{
    var neg;

    if ((binary.charAt(0) == 0))
        neg = true;
    else
        neg = false;

    var nullE = true;

    var nullF = true;

    var oneE = true;

    var strE = "";

    var x = 0;

    var exp;

    var man;

    var bias;

    switch (type)
    {

    case "LREAL":
        exp = 11;
        man = 52;
        bias = 1023;
        break;

    default:
        exp = 8;
        man = 23;
        bias = 127;
    }

    for (var i = 1; i <= exp; i++)
    {
        strE += binary.charAt(i);

        if (binary.charAt(i) != "0")
            nullE = false;

        if (binary.charAt(i) != "1")
            oneE = false;
    }

    var strF = "";

    for (var i = exp + 1; i <= exp + man; i++)
    {
        strF += binary.charAt(i);

        if (binary.charAt(i) != "0")
            nullF = false;
    }

    if (nullE && nullF)
        return ((!neg) ? "0": "-0");

    if (oneE && nullF)
        return ((!neg) ? "Infinity": "-Infinity");

    if (oneE && nullF)
        return "NaN";

    var exponent = b2i(strE) - bias;

    var fraction = 0;

    for (var i = 0; i < strF.length; ++i)
    {
        fraction = fraction + parseInt(strF.charAt(i)) * Math.pow(2, -(i + 1));
    }

    fraction = fraction + 1;
    var ret = Math.pow( - 1, binary.charAt(0)) * fraction * Math.pow(2, exponent);

    return ret;
}

function b2i(binary)
{
    var ret = 0;

    for (var i = 0; i < binary.length; ++i)
    {
        if (binary.charAt(i) == '1')
            ret = ret + Math.pow(2, (binary.length - i - 1));
    }

    return ret;
}