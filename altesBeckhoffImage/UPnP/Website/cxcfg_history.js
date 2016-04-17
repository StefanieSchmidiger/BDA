function Timer()
{
    this.obj = (arguments.length) ? arguments[0] : window;
    return this;
}

Timer.prototype.setInterval = function(func, ms)
{
    var i = Timer.getNew();
    var t = Timer.buildCall(this.obj, i, arguments);
    Timer.set[i].timer = window.setInterval(t, ms);
    return i;
}

Timer.prototype.setTimeout = function(func, ms)
{
    var i = Timer.getNew();
    Timer.buildCall(this.obj, i, arguments);
    Timer.set[i].timer = window.setTimeout("Timer.callOnce(" + i + ");", ms);
    return i;
}

Timer.set = [];
Timer.buildCall = function(obj, i, args)
{
    var t = "";
    Timer.set[i] = [];

    if (obj != window)
    {
        Timer.set[i].obj = obj;
        t = "Timer.set[" + i + "].obj.";
    }

    t += args[0] + "(";

    if (args.length > 2)
    {
        Timer.set[i][0] = args[2];
        t += "Timer.set[" + i + "][0]";

        for (var j = 1; (j + 2) < args.length; j++)
        {
            Timer.set[i][j] = args[j + 2];
            t += ", Timer.set[" + i + "][" + j + "]";
        }
    }

    t += ");";
    Timer.set[i].call = t;
    return t;
}

Timer.callOnce = function(i)
{
    if (!Timer.set[i])
        return;

    eval(Timer.set[i].call);

    Timer.set[i] = null;
}

Timer.getNew = function()
{
    var i = 0;

    while (Timer.set[i])
        i++;

    return i;
}

function Locator(propertyToUse, dividingCharacter)
{
    this.propertyToUse = propertyToUse;
    this.defaultQS = "-1";
    this.dividingCharacter = dividingCharacter;
}

Locator.prototype.getLocation = function()
{
    return eval(this.propertyToUse);
}

Locator.prototype.getHash = function()
{
    var url = this.getLocation();

    if (url.indexOf(this.dividingCharacter) > -1)
    {
        var url_elements = url.split(this.dividingCharacter);
        return url_elements[url_elements.length - 1];
    }
    else
    {
        return this.defaultQS;
    }
}

Locator.prototype.getHref = function()
{
    var url = this.getLocation();
    var url_elements = url.split(this.dividingCharacter)
                       return url_elements[0];
}

Locator.prototype.addLocation = function(new_qs)
{
    return this.getHref() + this.dividingCharacter + new_qs;
}

function AjaxIframesFixer()
{
    if (document.getElementById('ajaxnav'))
    {
        this.fixLinks('ajaxnav');

        this.locator = new Locator("document.frames['ajaxnav'].getLocation()", "?hash=");
        this.windowlocator = new Locator("window.location.href", "#");
        this.timer = new Timer(this);

        this.delayInit();
        // required or IE doesn't fire
    }
}

AjaxIframesFixer.prototype.fixLinks = function()
{
    var links = document.getElementsByTagName("A");

    for (var i = 0; i < links.length; i++)
    {
        links[i].setAttribute("href", fixLink(links[i].getAttribute("href"), 'ajaxnav'))
    }
}

AjaxIframesFixer.prototype.delayInit = function()
{
    this.timer.setTimeout("checkBookmark", 100, "");
}

AjaxIframesFixer.prototype.checkBookmark = function()
{
    this.checkWhetherChanged(0);
}

AjaxIframesFixer.prototype.checkWhetherChanged = function(location)
{
    if (this.locator.getHash() != location)
    {
        new Mutex(show_type,new ShowData(this.locator.getHash()),0);
    }

    this.timer.setTimeout("checkWhetherChanged", 100, this.locator.getHash());
}

function fixLink(href)
{
    if (document.is_ie)
    {
        if (href.indexOf("#") != -1)
        {
            var hash = href.substr(href.indexOf("#") + 1);
            return "javascript:document.getElementById('ajaxnav').setAttribute('src', 'position.asp?hash=" + hash + "');";
        }
        else
            return href;
    }
    else
        return href;
}

function AjaxUrlFixer()
{
    this.locator = new Locator("window.location.href", "#");
    this.timer = new Timer(this);
    this.checkWhetherChanged( - 1);
}

AjaxUrlFixer.prototype.checkWhetherChanged = function(location)
{
    if (this.locator.getHash() != location)
    {
        new Mutex(show_type,new ShowData(this.locator.getHash()),0);
    }

    this.timer.setTimeout("checkWhetherChanged", 50, this.locator.getHash());
}