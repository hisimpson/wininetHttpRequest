<%@page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8" %>
<%@page import="java.io.*"%>
<%
  request.setCharacterEncoding("UTF-8");
  String dest = "C:/test/println.txt";
 

  String user_name = request.getParameter("user_name");
  String user_id = request.getParameter("user_id");
  String user_address = request.getParameter("user_address");
  String message = ""; // 결과 메시지
  String script = "";  // 결과 후 실행할 javascript 

  message = "성공적으로 회원가입 되었습니다.";
  StringWriter str = new StringWriter();
  PrintWriter pw = new PrintWriter(str);

  pw.println("[이름] : "+user_name);
  pw.println("[아이디] : "+user_id);
  pw.println("[주소] : "+user_address);
  pw.println("---------------------------------------");

  FileWriter file = new FileWriter(dest,true); 
  System.out.println(str.toString());
  file.append(str.toString());
  file.close();
  
%>
<html>
<head>
	<script type="text/javascript">
	</script>
<meta http-equiv="Content-Type" content="text/html; charset=EUC-KR">
<title>Insert title here</title>

</head>
<body>
	<%=str%>
</body>
</html>