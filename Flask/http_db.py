import sqlite3
import os
from flask_cors import CORS
from flask import Flask

app = Flask(__name__)
# r'/*' 是通配符，让本服务器所有的 URL 都允许跨域请求
CORS(app, resources=r'/*')

@app.route('/access',methods=["GET","POST"])
def forResponse():
    db_file = os.path.join(os.path.dirname(__file__), 'weather.db')
    # 初始数据:
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM Wdata  ORDER BY ostime DESC limit 1")
    # 使用 fetchone() 方法获取一条数据
    nowData = cursor.fetchone()
    cursor.close()
    conn.commit()
    conn.close()
    # print(type(nowData))
    print("实时数据：",nowData)  # 误差±10s

    newData = {"wendu": nowData[2], "shidu": nowData[3], "yuliang": nowData[4], "guangzhao": nowData[5]}
    print(newData)
    return newData

if __name__ == '__main__':
    app.run(port=5000, debug=False, threaded=True)  #processes = 8 在windows下无效