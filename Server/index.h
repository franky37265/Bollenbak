const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<style>
    body {
        background: #3366ff;
    }

    input[type=text],input[type=number], input[type=password],
    select {
        width: 100%;
        padding: 12px 20px;
        margin: 8px 0;
        display: inline-block;
        border: 1px solid #ccc;
        border-radius: 4px;
        box-sizing: border-box;
    }

    input[type=submit] {
        width: 100%;
        background-color: #3366ff;
        color: white;
        padding: 14px 20px;
        margin: 8px 0;
        border: none;
        border-radius: 4px;
        cursor: pointer;
    }


    input[type=submit]:hover {
        background-color: #3366ff;
    }
    button :hover{
        background-color: #3366ff;
    }

    div, .container {
        border-radius: 5px;
        background-color: #ffffff;
        padding: 20px;
        max-width: 1024px;
        display: flex;
        width: 512px;
        margin: auto;
    }
</style>

<body>
    <div class="container">
        <form method="get" action="/get">
            <h1>Instellingen: (SERVER)</h1>
            <p>My IP: {{OWNIP}}</p>
            <label for="SSID">SSID</label>
            <input type="text" id="SSID" name="SSID" placeholder="SSID" required >
           
            <label for="PASSWD">Password</label>
            <input type="password" id="PASSWD" name="PASSWD" placeholder="Password" required>

            <label for="ALMTIME">Alarmtijd</label>
            <input type="number" id="ALMTIME" name="ALMTIME" placeholder="Alarm tijd" required>

            <input type="submit" value="Opslaan">
        </form>
    </div>
    <div>
	    <form action="/reboot">
        <input type="submit" value="Reboot">
	    </form>
	</div>
<script>
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var text = JSON.parse(this.responseText);
      document.getElementById("SSID").value = text.STAssid;
      document.getElementById("PASSWD").value = text.STApasswd;      
      document.getElementById("ALMTIME").value = text.ALMTIME;
      //this.responseText;
    }
  };
  xhttp.open("GET", "settings", true);
  xhttp.send();
}
window.onload=getData();
</script>
</body>
</html>
)rawliteral";