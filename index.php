<!DOCTYPE HTML>
<html>
    <script src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js" type="text/javascript" charset="utf-8"></script>
<body>
    <span id="target1"></span><br>
    <span id="target2"></span><br>
    <span id="target3"></span><br>
    <span id="target4"></span><br>
    <span id="tstamp"></span>

    <br><br>
    <button id="startbutton" onclick="start()">Start Round!</button>
    <button id="resetbutton" onclick="reset()">Reset</button>

    <script type="text/javascript">
    var deviceID = "55ff6a065075555323151487";
    var accessToken = "db4bb1c624b52c47b322b4fad56e071abcfa7a1d";
    var startFunc = "startround";
    var resetFunc = "reset";
    function start() {
        document.getElementById("startbutton").disabled=true;
        document.getElementById("target1").innerHTML = "Waiting for data...";
        var eventSource = new EventSource("https://api.spark.io/v1/devices/" + deviceID + "/events/?access_token=" + accessToken);
        var postURL = "https://api.spark.io/v1/devices/" +deviceID + "/" + startFunc + "/";
        $.post( postURL, { access_token: accessToken });

        eventSource.addEventListener('open', function(e) {
            console.log("Opened!"); },false);

        eventSource.addEventListener('error', function(e) {
            console.log("Errored!"); },false);

        eventSource.addEventListener('round/target1', function(e) {
            var parsedData1 = JSON.parse(e.data);
            var tempSpan1 = document.getElementById("target1");
            var tsSpan1   = document.getElementById("tstamp");
            tempSpan1.innerHTML = " Target 1: " + parsedData1.data + " seconds";
            tempSpan1.style.fontSize = "28px";
        }, false);
        eventSource.addEventListener('round/target2', function(e) {
            var parsedData2 = JSON.parse(e.data);
            var tempSpan2 = document.getElementById("target2");
            var tsSpan2   = document.getElementById("tstamp");
            tempSpan2.innerHTML = " Target 2: " + parsedData2.data + " seconds";
            tempSpan2.style.fontSize = "28px";
        }, false);
        eventSource.addEventListener('round/target3', function(e) {
            var parsedData3 = JSON.parse(e.data);
            var tempSpan3 = document.getElementById("target3");
            var tsSpan3   = document.getElementById("tstamp");
            tempSpan3.innerHTML = " Target 3: " + parsedData3.data + " seconds";
            tempSpan3.style.fontSize = "28px";
        }, false);
        eventSource.addEventListener('round/target4', function(e) {
            var parsedData4 = JSON.parse(e.data);
            var tempSpan4 = document.getElementById("target4");
            var tsSpan4   = document.getElementById("tstamp");
            tempSpan4.innerHTML = " Target 4: " + parsedData4.data + " seconds";
            tempSpan4.style.fontSize = "28px";
            tsSpan.innerHTML = "At timestamp " + parsedData1.published_at;
            tsSpan.style.fontSize = "9px";
        }, false);
        document.getElementById("resetbutton").disabled=false;
    }
     function reset() {
        //document.getElementById("resetbutton").disabled=true;
        document.getElementById("target1").innerHTML = "";
        document.getElementById("target2").innerHTML = "";
        document.getElementById("target3").innerHTML = "";
        document.getElementById("target4").innerHTML = "";

        var resetURL = "https://api.spark.io/v1/devices/" +deviceID + "/" + resetFunc + "/";
        $.post( resetURL, { access_token: accessToken });
        document.getElementById("startbutton").disabled=false;
     }
    </script>
</body>
</html>
