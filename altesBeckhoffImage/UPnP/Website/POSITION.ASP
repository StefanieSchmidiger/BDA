<%
Dim Var1, Var2
Var1 = Request.QueryString("hash")
Var2 = "http://" & Request.ServerVariables("SERVER_NAME") & Request.ServerVariables("URL") & "?" & Request.ServerVariables("QUERY_STRING")
%>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
	
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
        <title>CxConfig</title>
	<script type="text/javascript">
		function getLocation() {
			return '<%= Var2 %>';
		}
	</script>
</head>

<body>
hash: [<% = Var1 %>]
<br>
self: [<% = Var2 %>]
</body>
</html>
