var version = "1.0.4.12";
var b64s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
var transportarr = ["TCP/IP", "IIO LIGHTBUS", "PROFIBUS DP",
                    "PCI/ISA BUS", "ADS-UDP", "FATP-UDP", "COM-PORT", "USB", "CAN OPEN",
                    "DEVICE NET", "SSB", "SOAP"];

var g_db_sr = "";
var g_debug = false;
var g_debug_max_size = 10000;
var requests = new Requests();

var MODULETYPE_ACCESSCTRL = 0x0001;
var MODULETYPE_NIC = 0x0002;
var MODULETYPE_TIME = 0x0003;
var MODULETYPE_USERMGMT = 0x0004;
var MODULETYPE_RAS = 0x0005;
var MODULETYPE_FTP = 0x0006;
var MODULETYPE_SMB = 0x0007;
var MODULETYPE_TWINCAT = 0x0008;
var MODULETYPE_DATASTORE = 0x0009;
var MODULETYPE_SOFTWARE = 0x000A;
var MODULETYPE_CPU = 0x000B;
var MODULETYPE_MEMORY = 0x000C;
var MODULETYPE_LOGFILE = 0x000D;
var MODULETYPE_FIREWALLCE = 0x000E;
var MODULETYPE_FW_RULES = 0x000F;
var MODULETYPE_FSO = 0x0010;
var MODULETYPE_REGISTRY = 0x0011;
var MODULETYPE_PLC = 0x0012;
var MODULETYPE_DISPLAY = 0x0013;
var MODULETYPE_EWF = 0x0014;
var MODULETYPE_FBWF = 0x0015;
var MODULETYPE_REGFILTER = 0x0016;
var MODULETYPE_SILICONDRIVE = 0x0017;
var MODULETYPE_OS = 0x0018;
var MODULETYPE_RAID = 0x0019;
var MODULETYPE_MISC = 0x0100;
var MODULETYPE_CUSTOMERPAGE = 0x0201;

var g_method;
var g_ndata = null;
var g_interval = null;
var mod = null;
var mod_list = [];
var mod_count = 0;
var req;
var lreq;
var retries = 10;
var tries = 0;
var bidx = 0;
var errors = 0;
var maxerrors = 6;
var g_executionTimeout = 10000;

var fixDocumentSizeLoopID = undefined;

var is_open = [];
var openpos = 0;

var currenttemp = [];
var general_info = new GeneralInfo();
var lastmenu = -1;
var rebootindex = -1;
var folderindex = -1;
var browseindex = -1;
var systemdsindex = -1;
var folderparent = "";
var dragapproved = false;
var cancelreturn = false;
var buffer_size = 0x2000;
var full_buffer = [];
var full_buffer_param = "";
var is_rebooting = false;

var agt = navigator.userAgent.toLowerCase();
document.is_ie = ((agt.indexOf("msie") != -1));

var boxobj;

document.onmousedown = mouse_down;
document.onmousemove = mouse_move;
document.onmouseup = mouse_up;
var overbox = false;

var imgSrcArr = [];
imgSrcArr[0] = "IMAGES/ADDFAV.GIF";
imgSrcArr[1] = "IMAGES/BAR.GIF";
imgSrcArr[2] = "IMAGES/BAR.JPG";
imgSrcArr[3] = "IMAGES/beckhoff_com.gif";
imgSrcArr[4] = "IMAGES/beckhoff_com_over.gif";
imgSrcArr[5] = "IMAGES/beckhoff_de.gif";
imgSrcArr[6] = "IMAGES/beckhoff_de_over.gif";
imgSrcArr[7] = "IMAGES/bg_frame_top.gif";
imgSrcArr[8] = "IMAGES/BU_INDEX.GIF";
imgSrcArr[9] = "IMAGES/bu_index_over.gif";
imgSrcArr[10] = "IMAGES/CALENDAR.GIF";
imgSrcArr[11] = "IMAGES/CHECK.GIF";
imgSrcArr[12] = "IMAGES/close_icon.jpg";
imgSrcArr[13] = "IMAGES/CX1000.JPG";
imgSrcArr[14] = "IMAGES/delete.gif";
imgSrcArr[15] = "IMAGES/HEADER.JPG";
imgSrcArr[16] = "IMAGES/INFO.GIF";
imgSrcArr[17] = "IMAGES/LM.GIF";
imgSrcArr[18] = "IMAGES/LOGO.GIF";
imgSrcArr[19] = "IMAGES/LOGO.JPG";
imgSrcArr[20] = "IMAGES/LU.GIF";
imgSrcArr[21] = "IMAGES/MU.GIF";
imgSrcArr[22] = "IMAGES/navbg.jpg";
imgSrcArr[23] = "IMAGES/OZ.GIF";
imgSrcArr[24] = "IMAGES/PM.GIF";
imgSrcArr[25] = "IMAGES/SE.GIF";
imgSrcArr[26] = "IMAGES/SELECT.GIF";
imgSrcArr[27] = "IMAGES/SLIDER.GIF";
imgSrcArr[28] = "IMAGES/SLIDER.JPG";
imgSrcArr[29] = "IMAGES/T.GIF";
imgSrcArr[30] = "IMAGES/Time.gif";
imgSrcArr[31] = "IMAGES/UNUSED.GIF";
imgSrcArr[32] = "IMAGES/UNUSED.JPG";
imgSrcArr[33] = "IMAGES/edit.gif";


//structs


function GeneralInfo()
{
    this.hostname = "";
    this.hardware = "";
    this.os = "";
}

function EvalData(k, c_val, error)
{
    this.k = k;
    this.c_val = c_val;
    this.error = error;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////// */
// for appended functions: displaytype=function, subindex=subindex, len=offset,
// array=0,1,2 (no, array, extended array)
function Variable(index, subindex, array, name, datatype, displaytype, editable, len, need)
{
    this.index = index;
    this.array = array;
    this.name = name;
    this.datatype = datatype;
    this.editable = editable;
    this.subindex = subindex;
    this.len = len;

    if (datatype != "func" && displaytype == "")
        this.displaytype = datatype;
    else
        this.displaytype = displaytype;

    this.need = need;

    this.disable = false;
}

function FuncVar(name, datatype, displaytype, type, len, disable, sidx)
{
    this.name = name;
    this.datatype = datatype;
    this.type = type;
    this.displaytype = displaytype;
    this.length = len;
    this.disable = disable;
    this.sidx = sidx;
}

function SrData(method, loop, sr)
{
    this.method = method;
    this.loop = loop;
    this.sr = sr;

    this.mutexIndex = 0;
}

function QueryData (index, subindex, flag, len, type, data, id, loop)
{
    this.index = index;
    this.subindex = subindex;
    this.flag = flag;
    this.len = len;
    this.type = type;
    this.data = data;
    this.id = id;
    this.loop = loop;
    this.mutexIndex = 0;

    this.name = "";
    this.i = "";
    this.refresh = false;
}

function FolderData(folder, parent, type, qid)
{
    this.folder = folder;
    this.parent = parent;
    this.type = type;
    this.qid = qid;

    this.mutexIndex = 0;
}

function ShowData(value)
{
    this.value = value;
    this.mutexIndex = 0;
}

function Function(name, index, variables, show, refresh)
{
    this.name = name;
    this.index = index;
    this.variables = variables;
    this.show = show;
    this.refresh = refresh;
}


function editItem(i, val, id)
{
    this.i = i;
    this.val = val;
    this.id = id;
}



//prototypes
String.prototype.startsWith = function(str)
{
    return (this.match("^" + str) == str);
}

Array.prototype.remove=function(s)
{
    var i = this.indexOf(s);
    if (this.indexOf(s) != -1)
        this.splice(i, 1);
}