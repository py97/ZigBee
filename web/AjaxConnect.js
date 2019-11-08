/**
 * 连接服务器
 * @param url   服务器地址
 * @param success   设置传送成功后弹出的信息
 */

AjaxConnect = function (url,success) {
    // 异步对象
    var ajax = new XMLHttpRequest();
    ajax.open('post', url,true);
    // 需要设置请求报文
    ajax.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    ajax.send();

    // 当readyState改变时
    ajax.onreadystatechange = function () {
        // 在事件中 获取数据 并修改界面显示
        if (ajax.readyState == 4) {
            // console.log(ajax.responseText);
            if (ajax.status == 200) {
                console.log("服务器响应成功！");
                var sucResult = ajax.responseText;
                console.log(sucResult);
                success(sucResult);
                // 将 数据 让 外面可以使用
                return ajax.responseText;
            } else
                alert("服务器响应失败！");
        }
    }
}