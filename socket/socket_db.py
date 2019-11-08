import sqlite3
import time
import socket
import struct
import json
import re

SendBuf = [0x3A,0x00,0xFF,0x01,0xC4,0x23]
NodeData = []

def mmain():
    try:
        # print(request.args)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('192.168.0.100', 33333))
        req = struct.pack("%dB" % (len(SendBuf)), *SendBuf)
        sock.send(req)
        print("连接发送成功")

        RxBuf = sock.recv(1024)
        print("接收成功！接收到的数据：")
        print(RxBuf)
        start3A = ord(RxBuf[0:1])
        if start3A == 58:  # 判断第一个是否为0x3A
            # print("第1个为0x3A")

            # 解析数据
            if RxBuf[3] == 0x01:
                # print("第4个为0x01")

                # 光照强度
                Light_High = RxBuf[12]
                Light_Low = RxBuf[13]

                l_high = bin(Light_High).replace('0b', '')
                # print(l_high)

                l_low = bin(Light_Low).replace('0b', '')
                while len(l_low) != 8:
                    l_low = '0' + l_low
                # print(l_low)

                light_er = l_high + '' + l_low
                # print(light_er)
                light_shi = int(light_er, 2)
                # print(light_shi)

                obj = {"wendu": RxBuf[4], "shidu": RxBuf[5], "yuliang": RxBuf[8], "guangzhao": light_shi}
                print(obj)
                # jsonobj = json.dumps(obj, ensure_ascii=False, indent=4)
                # print("这是温度：",obj["wendu"])
                # return jsonobj

                # 连接到SQLite数据库，数据库文件是weather.db
                # 如果文件不存在，会自动在当前目录创建:
                conn = sqlite3.connect('weather.db')
                # 创建一个Cursor:
                mydb = conn.cursor()

                # 判断表是否存在,若不存在则创建表
                if table_exists(mydb, 'Wdata') == 0:
                    # 执行一条SQL语句，创建Wdata表:
                    mydb.execute('create table Wdata (nowtime varchar(20) , ostime varchar(30) primary key, '
                                 'wendu varchar(10), shidu varchar(10),'
                                 'yuliang varchar(10), guangzhao varchar(16))')

                nowTime = time.strftime('%Y/%m/%d/%H:%M:%S ', time.localtime(time.time()))
                mydb.execute('insert into Wdata (nowtime, ostime ,wendu, shidu, yuliang, guangzhao) '
                             'values (\'%s\',\'%s\', \'%s\', \'%s\', \'%s\', \'%s\')'
                             % (nowTime, time.time(), obj["wendu"], obj["shidu"], obj["yuliang"], obj["guangzhao"]))

                mydb.close()    # 关闭Cursor
                conn.commit()   # 提交事务
                conn.close()    # 关闭Connection

        else:
            return "查询wifi数据失败，请您稍后再查询"

        sock.close()  # 关闭socket连接

    except Exception as e:
        print(e)


def table_exists(con, table_name):  # 这个函数用来判断表是否存在
    sql = "select name from sqlite_master where type='table' order by name"
    con.execute(sql)
    tables = [con.fetchall()]
    table_list = re.findall('(\'.*?\')', str(tables))
    table_list = [re.sub("'", '', each) for each in table_list]
    if table_name in table_list:
        print("table exist")
        return 1  # 存在返回1

    else:
        print("table not exist")
        return 0  # 不存在返回0

if __name__ == '__main__':
    while 1:
        time.sleep(20)
        mmain()