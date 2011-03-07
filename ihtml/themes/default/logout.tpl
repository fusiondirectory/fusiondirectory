<body style="background-color: white;background-image:none;">

<form action='index.php' method='post' name='mainform'>
<div style="margin-left:10%; margin-right:10%; margin-top:5%; border:2px solid red;padding-left:10px;padding-right:10px;padding-top:5px;padding-bottom:20px;">
 <h1>{t}Your GOsa session has expired!{/t}</h1>
 <p>
 {t}The last interaction with the GOsa web interface has been some time ago in the past. For security reasons, the session has been closed. To continue with administrative tasks, please sign in again.{/t}
 </p>
 <br>
 <center><input type="submit" name="dummy" value="{t}Sign in again{/t}"></center>
</div>
</form>

</body>
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  focus_field('dummy');
  -->
</script>

</html>
