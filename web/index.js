window.onload = function(){
    AjaxConnect("http://192.168.0.112:5000/index",
        success = function (respon) {
        respon = JSON.parse(respon);
        console.log(respon);
        var draw = document.getElementById("wendu");
        var draw1 = document.getElementById("shidu");
        var draw2 = document.getElementById("yuliang");
        var draw3 = document.getElementById("guangzhao");
        if(!draw.getContext && !draw1.getContext && !draw2.getContext && !draw3.getContext){
            return false;
        }

            //温度进度条
            var context=draw.getContext('2d'), centerX=draw.width/2,
                centerY=draw.height/2,
                angle= (Math.abs(respon.wendu))/100, //占的百分数
                color=[], //"#e5e5e5","red","#F00"
                font="40px Arial";
            speed = 0; //从度数开始70---》表示从70度开始
            context.lineCap="round";  //square 平角的帽

            //湿度进度条
            var context1=draw1.getContext('2d'), centerX1=draw1.width/2,
                centerY1=draw1.height/2,
                angle1= (respon.shidu)/100, //占的百分数
                color1=[], //"#e5e5e5","red","#F00"
                font1="40px Arial";
            speed1 = 0; //从度数开始70---》表示从70度开始
            context1.lineCap="round";  //square 平角的帽

            //雨量进度条
            var context2=draw2.getContext('2d'), centerX2=draw2.width/2,
                centerY2=draw2.height/2,
                angle2= (respon.yuliang)/255, //占的比例
                color2=[], //"#e5e5e5","red","#F00"
                font2="40px Arial";
            speed2 = 0; //从度数开始70---》表示从70度开始
            context2.lineCap="round";  //square 平角的帽

            //光照进度条
            var context3=draw3.getContext('2d'), centerX3=draw3.width/2,
                centerY3=draw3.height/2,
                angle3= (respon.guangzhao)/550, //占的比例
                color3=[], //"#e5e5e5","red","#F00"
                font3="40px Arial";
            speed3 = 0; //从度数开始70---》表示从70度开始
            context3.lineCap="round";  //square 平角的帽

            //绘制温度外圈的圆
            function blueCircle(){
                context.save();//save() 方法保存当前图像状态的一份拷贝。
                context.strokeStyle = color[0] || "#e5e5e5"; //设置描边样式
                context.lineWidth = 15; //设置线宽
                context.beginPath();//路径开始
                //context.arc(centerX, centerY, 100 , -Math.PI/2, -Math.PI/2 +n*rad, false);
                //用于绘制圆弧context.arc(x坐标，y坐标，半径，起始角度，终止角度，顺时针/逆时针)
                context.arc(centerX,centerY,centerX-5,0,2*Math.PI,false);
                /*context.moveTo(210,55);
                context.arc(55,55,20,0,2*Math.PI,false);*/
                context.stroke();
                context.closePath(); //路径结束
                context.restore();//save() 方法把当前状态的一份拷贝压入到一个保存图像状态的栈中。这就允许您临时地改变图像状态，然后，通过调用 restore() 来恢复以前的值。
            }

            //绘制湿度外圈的圆
            function blueCircle1(){
                context1.save();//save() 方法保存当前图像状态的一份拷贝。
                context1.strokeStyle = color1[0] || "#e5e5e5"; //设置描边样式
                context1.lineWidth = 15; //设置线宽
                context1.beginPath();//路径开始
                //context.arc(centerX, centerY, 100 , -Math.PI/2, -Math.PI/2 +n*rad, false);
                //用于绘制圆弧context.arc(x坐标，y坐标，半径，起始角度，终止角度，顺时针/逆时针)
                context1.arc(centerX1,centerY1,centerX1-5,0,2*Math.PI,false);
                /*context.moveTo(210,55);
                context.arc(55,55,20,0,2*Math.PI,false);*/
                context1.stroke();
                context1.closePath(); //路径结束
                context1.restore();//save() 方法把当前状态的一份拷贝压入到一个保存图像状态的栈中。这就允许您临时地改变图像状态，然后，通过调用 restore() 来恢复以前的值。
            }

            //绘制雨量外圈的圆
            function blueCircle2(){
                context2.save();//save() 方法保存当前图像状态的一份拷贝。
                context2.strokeStyle = color2[0] || "#e5e5e5"; //设置描边样式
                context2.lineWidth = 15; //设置线宽
                context2.beginPath();//路径开始
                //context.arc(centerX, centerY, 100 , -Math.PI/2, -Math.PI/2 +n*rad, false);
                //用于绘制圆弧context.arc(x坐标，y坐标，半径，起始角度，终止角度，顺时针/逆时针)
                context2.arc(centerX2,centerY2,centerX2-5,0,2*Math.PI,false);
                /*context.moveTo(210,55);
                context.arc(55,55,20,0,2*Math.PI,false);*/
                context2.stroke();
                context2.closePath(); //路径结束
                context2.restore();//save() 方法把当前状态的一份拷贝压入到一个保存图像状态的栈中。这就允许您临时地改变图像状态，然后，通过调用 restore() 来恢复以前的值。
            }

            //绘制光照外圈的圆
            function blueCircle3(){
                context3.save();//save() 方法保存当前图像状态的一份拷贝。
                context3.strokeStyle = color3[0] || "#e5e5e5"; //设置描边样式
                context3.lineWidth = 15; //设置线宽
                context3.beginPath();//路径开始
                //context.arc(centerX, centerY, 100 , -Math.PI/2, -Math.PI/2 +n*rad, false);
                //用于绘制圆弧context.arc(x坐标，y坐标，半径，起始角度，终止角度，顺时针/逆时针)
                context3.arc(centerX3,centerY3,centerX3-5,0,2*Math.PI,false);
                /*context.moveTo(210,55);
                context.arc(55,55,20,0,2*Math.PI,false);*/
                context3.stroke();
                context3.closePath(); //路径结束
                context3.restore();//save() 方法把当前状态的一份拷贝压入到一个保存图像状态的栈中。这就允许您临时地改变图像状态，然后，通过调用 restore() 来恢复以前的值。
            }

            //绘制温度红色外圈
            function redCircle(n){
                context.save();
                if (respon.wendu <= 0){
                    context.strokeStyle = color[1] || "Aqua"; //设置描边样式
                }
                else context.strokeStyle = color[2] || "red"; //设置描边样式
                context.lineWidth = 15; //设置线宽
                context.beginPath();
                context.arc(centerX, centerY, centerX-5 ,  -Math.PI /2, (n * 3.6 - 90) * Math.PI / 180, false);
                context.stroke();
                context.closePath();
                context.restore();
            }

            //绘制湿度红色外圈
            function redCircle1(n){
                context1.save();
                context1.strokeStyle = color2[1] || "green"; //设置描边样式
                context1.lineWidth = 15; //设置线宽
                context1.beginPath();
                context1.arc(centerX1, centerY1, centerX1-5 ,  -Math.PI /2, (n * 3.6 - 90) * Math.PI / 180, false);
                context1.stroke();
                context1.closePath();
                context1.restore();
            }

            //绘制雨量红色外圈
            function redCircle2(n){
                context2.save();
                context2.strokeStyle = color2[1] || "blue"; //设置描边样式
                context2.lineWidth = 15; //设置线宽
                context2.beginPath();
                context2.arc(centerX2, centerY2, centerX2-5 ,  -Math.PI /2, (n * 3.6 - 90) * Math.PI / 180, false);
                context2.stroke();
                context2.closePath();
                context2.restore();
            }

            //绘制光照红色外圈
            function redCircle3(n){
                context3.save();
                context3.strokeStyle = color3[1] || "orange"; //设置描边样式
                context3.lineWidth = 15; //设置线宽
                context3.beginPath();
                context3.arc(centerX3, centerY3, centerX3-5 ,  -Math.PI /2, (n * 3.6 - 90) * Math.PI / 180, false);
                context3.stroke();
                context3.closePath();
                context3.restore();
            }

            //温度百分比文字绘制
            function text(n){
                context.save(); //save和restore可以保证样式属性只运用于该段canvas元素
                //context.strokeStyle = "#F00"; //设置描边样式
                //context.fillStyle='#f00';//文字颜色
                context.beginPath();
                context.font =font || "40px Arial"; //设置字体大小和字体
                context.fillStyle=color[2] || "yellow";
                context.textAlign='center';//文本程度对齐方法
                context.textBaseline='middle';//文本垂曲标的目的，基线位置
                //绘制字体，并且指定位置
                context.fillText("温度："+n.toFixed(0)+"℃", centerX, centerY);
                context.stroke(); //执行绘制
                context.closePath();
                context.restore();
            }

            //湿度百分比文字绘制
            function text1(n){
                context1.save(); //save和restore可以保证样式属性只运用于该段canvas元素
                //context.strokeStyle = "#F00"; //设置描边样式
                //context.fillStyle='#f00';//文字颜色
                context1.beginPath();
                context1.font =font1 || "40px Arial"; //设置字体大小和字体
                context1.fillStyle=color1[2] || "yellow";
                context1.textAlign='center';//文本程度对齐方法
                context1.textBaseline='middle';//文本垂曲标的目的，基线位置
                //绘制字体，并且指定位置
                context1.fillText("湿度："+n.toFixed(0)+"%", centerX1, centerY1);
                context1.stroke(); //执行绘制
                context1.closePath();
                context1.restore();
            }

            //雨量百分比文字绘制
            function text2(n){
                context2.save(); //save和restore可以保证样式属性只运用于该段canvas元素
                //context.strokeStyle = "#F00"; //设置描边样式
                //context.fillStyle='#f00';//文字颜色
                context2.beginPath();
                context2.font =font2 || "40px Arial"; //设置字体大小和字体
                context2.fillStyle=color2[2] || "yellow";
                context2.textAlign='center';//文本程度对齐方法
                context2.textBaseline='middle';//文本垂曲标的目的，基线位置
                //绘制字体，并且指定位置
                context2.fillText("雨量："+n.toFixed(0)+"%", centerX2, centerY2);
                context2.stroke(); //执行绘制
                context2.closePath();
                context2.restore();
            }

            //光照百分比文字绘制
            function text3(n){
                context3.save(); //save和restore可以保证样式属性只运用于该段canvas元素
                //context.strokeStyle = "#F00"; //设置描边样式
                //context.fillStyle='#f00';//文字颜色
                context3.beginPath();
                context3.font =font3 || "40px Arial"; //设置字体大小和字体
                context3.fillStyle=color3[2] || "yellow";
                context3.textAlign='center';//文本程度对齐方法
                context3.textBaseline='middle';//文本垂曲标的目的，基线位置
                //绘制字体，并且指定位置
                context3.fillText("光照："+respon.guangzhao+" lx", centerX3, centerY3);
                context3.stroke(); //执行绘制
                context3.closePath();
                context3.restore();
            }

            //自己一直调用自己动画循环
            var timer=null;
            var timer1=null;
            var timer2=null;
            var timer3=null;

            //温度定时器
            (function drawFrame(){
                timer=setTimeout(drawFrame,6);
                context.clearRect(0, 0, draw.width, draw.height);
                blueCircle();
                redCircle(speed);
                text(speed);
                if(speed > angle*100)  clearTimeout(timer); //清除定时器
                speed += 0.6;

                // TestGetCode(context,draw,angle,blueCircle(),redCircle(speed),speed);
                // TestGetCode(context1,draw1,angle1,blueCircle1(),redCircle1(speed1),speed1);
            }());

            //湿度定时器
            (function drawFrame1(){
                timer1=setTimeout(drawFrame1,6);
                context1.clearRect(0, 0, draw1.width, draw1.height);
                blueCircle1();
                redCircle1(speed1);
                text1(speed1);
                if(speed1 > angle1*100)  clearTimeout(timer1); //清除定时器
                speed1 += 0.6;

                // TestGetCode(context,draw,angle,blueCircle(),redCircle(speed),speed);
                // TestGetCode(context1,draw1,angle1,blueCircle1(),redCircle1(speed1),speed1);
            }());

            //雨量定时器
            (function drawFrame2(){
                timer2=setTimeout(drawFrame2,6);
                context2.clearRect(0, 0, draw2.width, draw2.height);
                blueCircle2();
                redCircle2(speed2);
                text2(speed2);
                if(speed2 > angle2*100)  clearTimeout(timer2); //清除定时器
                speed2 += 0.6;

                // TestGetCode(context,draw,angle,blueCircle(),redCircle(speed),speed);
                // TestGetCode(context1,draw1,angle1,blueCircle1(),redCircle1(speed1),speed1);
            }());

            //光照定时器
            (function drawFrame3(){
                timer3=setTimeout(drawFrame3,6);
                context3.clearRect(0, 0, draw3.width, draw3.height);
                blueCircle3();
                redCircle3(speed3);
                text3(speed3);
                if(speed3 > angle3*100)  clearTimeout(timer3); //清除定时器
                speed3 += 0.6;

                // TestGetCode(context,draw,angle,blueCircle(),redCircle(speed),speed);
                // TestGetCode(context1,draw1,angle1,blueCircle1(),redCircle1(speed1),speed1);
            }());
    });
}

