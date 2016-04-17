<%@ Language=VBScript %>
<html>
<head>
<title>Welcome</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>

<body bgcolor="#FFFFFF" text="#000000">
<%	
	'create TCSript-Object 
	Set TCSCRIPT = CreateObject("TCSCRIPT.TcScriptSync")
	
	'connect to ADS device 
	'NOTE: For local ADS on CE-device you have to use NetId 127.0.0.1.1.1 
	Call TCSCRIPT.ConnectTo("127.0.0.1.1.1", 801)
	
	'get values of memory variables MW4 / MW6
	Dim Var1, Var2
	Var1 = TCSCRIPT.ReadInt16(&H4020&, 4)
	Var2 = TCSCRIPT.ReadInt16(&H4020&, 6)

%>
<br>
<B>BECKHOFF CE-device</B> <br>
<B>Status variables</B> <br>


<br>
<% = check %>
<table width="209" border="1" cellspacing="0" cellpadding="0" height="30">
  <tr> 
    <td width="40" align="center" valign="middle">Mode</td>
    <td align="center" valign="middle"> 
      <% = Var1 %>
      </td>
  </tr>
  <tr> 
    <td align="center" valign="middle" height="2">Speed</td>
    <td height="2" align="center" valign="middle"> 
      <% = Var2 %>
      </td>
  </tr>
</table>
</body>
</html>
