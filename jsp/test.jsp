<%@page import="java.io.*"%>
<%@ page language="java" contentType="text/html; charset=utf-8" pageEncoding="utf-8"%>
<!DOCTYPE html">
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<title>Jsp save file test</title>
</head>
<body>

<%!

%>

<%
    request.setCharacterEncoding("UTF-8");
	String id = request.getParameter("id");
	String password = request.getParameter("password");
	String name = request.getParameter("name");
	String title = request.getParameter("title");
	String content = request.getParameter("content");
	
	String filename = id+".txt";
	PrintWriter writer = null;
	
	try{
		String filePath = application.getRealPath("/bbs/" + filename);
		writer = new PrintWriter(filePath);
		writer.printf("아이디: %s %n",id);
		writer.printf("비밀번호 : %s %n",password);
		writer.printf("이름 : %s %n", name);
		writer.printf("제목 : %s %n", title);
		writer.printf("내용 : %s %n", content);
		out.println("성공적으로 글이 저장되었습니다.");
	}
	catch(IOException ioe){
		out.println("파일로 글을 입력할 수 없습니다.");
	}
	finally {
		try{
		}
		catch(Exception e) {
			
		}
	}
		
%></p>
</body>
</html>
