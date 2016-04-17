<%@ language="VBScript" %>
<%
  dim url
  dim uuid
  dim req
  dim XmlDoc
  uuid = Request.Querystring("uuid")
  url = "http://localhost:2869/upnphost/udhisapi.dll?control=uuid:" & uuid & "+urn:beckhoff.com:serviceId:cxconfig"

  dim totalBytes
  totalBytes = Request.TotalBytes
  dim data
  data = Request.BinaryRead(totalBytes)
  dim str
  str = BinaryToString(data)

  Set req = Server.CreateObject("Microsoft.XMLHTTP")
  Set XmlDoc = Server.CreateObject("MSXML2.DomDocument") 

  if str <> "" then
  XmlDoc.loadXML str
  str = ""
  end if
  
  query Request.ServerVariables("HTTP_SOAPAction"), XmlDoc, totalBytes
  
  Function BinaryToString(Binary)
    Dim i, str
    For i = 1 To LenB(Binary)
      str = str & Chr(AscB(MidB(Binary, i, 1)))
    Next
    BinaryToString = str
  End Function
  
  Function query(soapAction, data, totalBytes)
      req.open "POST", url, False
      req.setRequestHeader "SOAPAction", soapAction
      req.setRequestHeader "Content-Type", "text/xml; charset=utf-8"
      req.setRequestHeader "Content-Length", totalBytes
      XmlDoc.async = False
      req.send data
      Response.ContentType="text/xml"
      Response.Write req.responseText
  End Function
%>
