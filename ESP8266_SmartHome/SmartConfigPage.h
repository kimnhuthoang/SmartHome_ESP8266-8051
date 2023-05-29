const char SmartConfigPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    
    <title>Config Wifi</title>
    <style>
        body{text-align: center;}
        .hoten{
            font-family: sans-serif;
            text-align: center;
            width: 340px;
            height: auto;
            border: 1px solid gray;
            border-radius: 10px;
            margin: 0 auto;
        }
        .hoten h2{color: #868787;}
        .hoten input{
            width: 280px;
            height: 30px;
            margin-bottom: 10px;
            border-radius: 5px;
            border: 1px solid gray;
            padding-left: 10px;
        }
        .hoten .input1{
            width: 130px; 
            height: 40px; 
            border-radius: 5px; 
            border: 1px solid gray;
            padding: 0 0;
            color: white;
            background-color: #45a049;
            font-size: 20px;
        }
        .hoten .input1:hover{
            background-color: #21b828;
            font-size: 24px;
        }
    </style>
</head>
<body>
    <h2 style="font-family: sans-serif">Smart Config Wifi - ESP8266</h2>
    <form action="/action">
        <div class="hoten">
            <h2>Nhập tên và mật khẩu Wifi</h2>
            <input type="text" id="ssid" name="ssid" placeholder="Tên Wifi"><br>
            <input type="password" id="pass" name="pass" placeholder="Mật Khẩu"><br>
            <input class="input1" type="submit" value="Gửi" onclick="thongbao('ssid','pass')">
        </div>
    </form>
    <footer style="margin-top:10px; font-size:16px; font-family:consolas;"><i>Design By <b>Kim Nhựt Hoàng</b></i></footer>
    <script>
        function thongbao(in1,in2){
            var ssid = document.getElementById(in1).value;
            var pass = document.getElementById(in2).value;
            alert('Xác nhận config, SSID: '+ssid + ' và PASS: '+pass);
        }
    </script>
</body>

</html>
)=====";
