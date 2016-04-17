function query_readwrite(queryData)
{
    var ndata = [], names = [], values = [];
    ndata = prepare_data(queryData.data, queryData.type, 0, ndata);

    debugln("Sending ReadWrite Query at index <b>" + queryData.index + "</b>,<b>" + queryData.subindex + "</b> with ID: <b>" + queryData.id + "</b>");

    names[0] = "netId";
    names[1] = "nPort";
    names[2] = "indexGroup";
    names[3] = "IndexOffset";
    names[4] = "cbRdLen";
    names[5] = "pwrData";

    values[0] = "";
    values[1] = "0";
    values[2] = "0";
    values[3] = get_mdpoffset(queryData.index, queryData.subindex, queryData.flag);
    values[4] = queryData.len;
    values[5] = encode_base64(ndata);

    requests.remove((queryData.loop?1:0));

    queryData.name = "ReadWrite";
    queryData.refresh = false;
    requests.add(queryData,(queryData.loop?1:0));

    query_full("ReadWrite", names, values, queryData.loop);
}

function query_readmultiple(queryData)
{
    var isStr = false, ci = "";

    if ((typeof queryData.id) == 'string')
        isStr = true;

    debugln("Sending Readmultiple Query ("+queryData.loop+") with " + queryData.index.length + " items (first index: <b>" + queryData.index[0] + "</b>,<b>" + queryData.subindex[0] + "</b>) and ID: <b>" + queryData.id + "</b>");


    requests.remove((queryData.loop?1:0));

    var data = [], j = 0;
    var clen = 0;

    for (var i = 0; i < queryData.index.length; i++)
    {
        if (queryData.index[i] != -1)
        {
            // add group (0)
            data[j++] = 0;
            data[j++] = 0;
            data[j++] = 0;
            data[j++] = 0;

            // add offset
            data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (0)), 10));
            data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (8)), 10));
            data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (16)), 10));
            data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (24)), 10));

            // add len
            data[j++] = to_byte(parseInt((queryData.len[i] >> (0)), 10));
            data[j++] = to_byte(parseInt((queryData.len[i] >> (8)), 10));
            data[j++] = to_byte(parseInt((queryData.len[i] >> (16)), 10));
            data[j++] = to_byte(parseInt((queryData.len[i] >> (24)), 10));

            if (queryData.data != -1)
                ci = queryData.data[i]
                     else
                         ci = data;


            var qData = new QueryData(queryData.index[i], queryData.subindex[i], queryData.flag[i], queryData.len[i], queryData.type[i], "", (isStr ? queryData.id : queryData.id[i]), queryData.loop);
            qData.mutexIndex = queryData.mutexIndex;
            qData.name = "ReadMultiple";
            qData.refresh = false;
            qData.i = ci;

            requests.add(qData,(qData.loop?1:0));

            clen++;
        }
    }
    
    if (requests.length((queryData.loop?1:0))==0)
    {
	 	Mutex.finish(queryData.mutexIndex);
	 	return;
	 }

    var names = [];
    names[0] = "netId";
    names[1] = "nPort";
    names[2] = "indexGroup";
    names[3] = "IndexOffset";
    names[4] = "cbRdLen";
    names[5] = "pwrData";

    var values = [];
    values[0] = "";
    values[1] = "0";
    values[2] = 61568;
    values[3] = clen;
    values[4] = clen;
    values[5] = encode_base64(data); debugln( encode_base64(data));
    query_full("ReadWrite", names, values, queryData.loop);
}

function query_writemultiple(queryData)
{
    debugln("Sending Writemultiple Query at index with " + queryData.index.length
            + " items (first index: <b>" + queryData.index[0] + "</b>,<b>"
            + queryData.subindex[0] + "</b>) and ID: <b>" + queryData.id + "</b>");
    var data= [], j = 0, isStr = false;

    if ((typeof queryData.id) == 'string')
        isStr = true;

    requests.remove((queryData.loop?1:0));

    var datatmp = [];

    for ( var i = 0; i < queryData.index.length; i++)
    {
        // add group (0)
        data[j++] = 0;
        data[j++] = 0;
        data[j++] = 0;
        data[j++] = 0;

        // add offset
        data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (0)), 10));
        data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (8)), 10));
        data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (16)), 10));
        data[j++] = to_byte(parseInt((get_mdpoffset(queryData.index[i],queryData.subindex[i], queryData.flag[i]) >> (24)), 10));


        if (queryData.type[i] == "string")
            queryData.len[i] = queryData.data[i].length;

        // add len
        data[j++] = to_byte(parseInt((queryData.len[i] >> (0)), 10));
        data[j++] = to_byte(parseInt((queryData.len[i] >> (8)), 10));
        data[j++] = to_byte(parseInt((queryData.len[i] >> (16)), 10));
        data[j++] = to_byte(parseInt((queryData.len[i] >> (24)), 10));

        var qData = new QueryData(queryData.index[i], queryData.subindex[i], queryData.flag[i], queryData.len[i], queryData.type[i], queryData.data[i], (isStr ? queryData.id : queryData.id[i]), queryData.loop);
        qData.mutexIndex = queryData.mutexIndex;
        qData.name = "WriteMultiple";
        qData.refresh = false;

        requests.add(qData,(qData.loop?1:0));

        datatmp = prepare_data(queryData.data[i], queryData.type[i], queryData.len[i], datatmp);
    }
    
   if (requests.length((queryData.loop?1:0))==0)
	 {
	 	Mutex.finish(queryData.mutexIndex);
	 	return;
	 }

    data = data.concat(datatmp);

    var names= [];
    names[0] = "netId";
    names[1] = "nPort";
    names[2] = "indexGroup";
    names[3] = "IndexOffset";
    names[4] = "cbRdLen";
    names[5] = "pwrData";

    var values= [];
    values[0] = "";
    values[1] = "0";
    values[2] = 61569;
    values[3] = queryData.index.length;
    values[4] = queryData.index.length;
    values[5] = encode_base64(data);

    query_full("ReadWrite", names, values, false);
}

function query_write(queryData)
{
    var ndata = [];
    ndata = prepare_data(queryData.data, queryData.type, 0, ndata);

    var names = [];
    names[0] = "netId";
    names[1] = "nPort";
    names[2] = "indexGroup";
    names[3] = "IndexOffset";
    names[4] = "pData";

    var values= [];
    values[0] = "";
    values[1] = "0";
    values[2] = "0";
    values[3] = get_mdpoffset(queryData.index, queryData.subindex, 0);
    values[4] = encode_base64(ndata);


    queryData.name = "Write";

    requests.remove((queryData.loop?1:0));
    requests.add(queryData,(queryData.loop?1:0));

    debugln("Sending Write Query at index <b>" + queryData.index + "</b>,<b>" + queryData.subindex + "</b>");
    query_full("Write", names, values, false);
}

function query_read(queryData)
{
    debugln("Sending Read Query at index <b>" + queryData.index + "</b>,<b>" + queryData.subindex + "</b> with ID: <b>" + queryData.id + "</b>");
    var names= [];
    names[0] = "netId";
    names[1] = "nPort";
    names[2] = "indexGroup";
    names[3] = "IndexOffset";
    names[4] = "cbRdLen";

    var values = [];
    values[0] = "";
    values[1] = "0";
    values[2] = "0";
    values[3] = get_mdpoffset(queryData.index, queryData.subindex, queryData.flag);
    values[4] = queryData.len;

    queryData.name = "Read";

    requests.remove((queryData.loop?1:0));
    requests.add(queryData,(queryData.loop?1:0));

    query_full("Read", names, values, false);
}

function query_sr(srData)
{
    g_db_sr = srData.sr;
    g_method = srData.method;

    try
    {
        if (srData.loop)
        {
            loadXMLDoc(device_url, lreq, processReqChangeLoop);
            lreq.setRequestHeader("SOAPAction","urn:beckhoff.com:service:cxconfig:1#" +srData.method);
            lreq.setRequestHeader("Content-Type","text/xml; charset=utf-8");
            lreq.setRequestHeader("Content-Length", srData.sr.length);
            lreq.send(srData.sr);
        }
        else
        {
            loadXMLDoc(device_url, req, processReqChange);
            req.setRequestHeader("SOAPAction","urn:beckhoff.com:service:cxconfig:1#" + srData.method);
            req.setRequestHeader("Content-Type", "text/xml; charset=utf-8");
            req.setRequestHeader("Content-Length", srData.sr.length);
            req.send(srData.sr);
        }
        if (mod && srData.method == "Write") //reset length of the table on write queries
        {
        	   mod.arrlens = [];
		  	}
        debugln(srData.method+" Query sent");
        if (srData.mutexIndex) Mutex.finish(srData.mutexIndex);
				
    }
    catch (e)
        // object is in use, try later
    {
        show_info("Error: " + (e.description != null ? e.description : e),"red");
        errors++;

        if (errors < maxerrors)
        {
            new Mutex(query_sr,srData,3,"",100);
        }
        else load_end();

        return;
    }
}

function query_full(method, names, values, loop)
{
    var sr = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
             + "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
             + "<s:Body><u:" + method
             + " xmlns:u=\"urn:beckhoff.com:service:cxconfig:1\">";

    if (names)
    {
        for ( var i = 0; i < names.length; i++)
        {
            if (names[i] != "Error" && names[i][0] != "-")
                sr += "<" + names[i] + ">" + values[i] + "</" + names[i] + ">";
        }
    }

    sr += "</u:" + method + "></s:Body></s:Envelope>";

    new Mutex(query_sr,new SrData(method,loop,sr),3);
}