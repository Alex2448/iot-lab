import datetime

import pytz
from dateutil import parser
import time

import pandas as pd
import tensorflow as tf
import numpy as np
import paho.mqtt.client as mqtt
import requests
from minio import Minio


def invoke_openwhisk():
    # AUTH_KEY = subprocess.check_output("wsk property get --auth", shell=True).split()[2]
    PARAMS = {'group9': 'aEVVai8cjm'}
    BLOCKING = 'false'
    RESULT = 'true'

    url = "https://138.246.236.111:31001/api/v1/web/guest/default/func-11-action-function-141"
    # user_pass = AUTH_KEY.split(':')
    requests.get(url, verify=False)

    #
    # url = "https://138.246.236.111:31001/api/v1/web/guest/default/func-11-action-testtracing-115"
    # resp = requests.get(url)
    # if resp.status_code == 200:
    #     return resp.content
    # else:
    #     return ConnectionError


def get_prediction():
    # get model from Minion
    minioAddress = "138.246.236.181:9900"

    # Connecting to MinIO
    minioClient = Minio(minioAddress,
                        access_key="group9",
                        secret_key="aEVVai8cjm",
                        secure=False)

    # model_object = minioClient.get_object('iot-group-9', 'model.h5')
    minioClient.fget_object('iot-group-9', 'model.h5', "model.h5")
    model = tf.keras.models.load_model("model.h5")
    last_value_object = minioClient.get_object('iot-group-9', 'last_value.csv')
    last_value = pd.read_csv(last_value_object)
    value = int(last_value['SensorValues'].iloc[0])  # float
    time = last_value['TimeStamps'].iloc[0]  # string
    lst = []
    lst1 = [value]
    lst.append(lst1)
    value_list = np.array(lst)
    prediction = model.predict(value_list).round(decimals=0)
    date_time_obj = parser.parse(time)

    # get the predicted time
    # tz = pytz.timezone('Europe/Berlin')
    # now = datetime.datetime.now(tz)
    # predict_time = now + datetime.timedelta(minutes=15)
    predict_time = date_time_obj.timestamp()
    predict_time = int(predict_time) * 1000
    return prediction, predict_time


# def on_connect(client, userdata, flags, rc):
#     if rc == 0:
#         print("Connected to MQTT Broker!")
#     else:
#         print("Failed to connect, return code %d\n", rc)


def send_to_broker(prediction, predict_time, broker_name, topic, Token=None):

    broker = broker_name
    port = 1883
    topic = topic
    client_id = '9'
    # Set Connecting Client ID
    client = mqtt.Client(client_id)
    if Token is not None:
        client.username_pw_set('JWT', Token)
    client.port = port
    client.connect(broker)

    if broker_name == "broker.emqx.io":
        msg = "{}".format(prediction)
    else:
        msg = "{{\"username\":\"group9\",\"device_id\":\"7\",\"prediction2\":\"{}\",\"timestamp\":\"{}\"}}".format(
            prediction, predict_time)
    result = client.publish(topic, msg)


def main():

    # correct run:
    while True:
        #invoke_openwhisk()
       # prediction, predict_time = get_prediction()
        prediction = 20
        tz = pytz.timezone('Europe/Berlin')
        now = datetime.datetime.now(tz)
        now = now.replace(tzinfo=None)
        predict_time = now
        predict_time = predict_time.timestamp()
        predict_time = int(predict_time) * 1000
        send_to_broker(prediction, predict_time, 'broker.emqx.io', f"/iotLabPrediction/{predict_time}")
        send_to_broker(prediction, predict_time, '138.246.236.107', "11_7",
                   "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2NTMzMTMyMTMsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMTFfNyJ9.onsrMI-GViz-9jTI3sdFcTj90E5B5KcPnyAXE49Tc3MyDjrUKj-rmXbCLthBhntyzhgTQU-GbUoNVqvPJJLzTomCsfqQexPdl-rNC0JVa3ws99au2eAyPRVBJUp0vSsHkzLJNNlU2soleJ3ZixMdVOT-Bl-N2kk4sQD5m-DFzBp1Cv5lS1iWNpg_vZE0fby0uZBEovW6T1QnXWn5Xc9DfKD1EMboBoPeK1JUSFzQHs5nI9-0hT3n-_abU2HZkYBCRXGWrhUQ-ukMNvjs5kwe8w1r-ULjx6QIPB04pmibQtOXTTPL8X5zVu20A8ymp1pQYnbbVaLCdg7y7WC0euTkZlKB9hIMF9lG2LjXt6bbpXKO_MbjpmZoQEZyRy_swzBd5A3gcNqb1rputI3aC2rZ5j7c1AEJfn6gDF78Qw2SeJFXsSElTEm4DtbSiZoQBvaImcmfEpyB_-zxlW1jwi15YRSUVQWbJxcziK3YU92WLIH-79mlOWu70QsRKznCNgXJSPLss5P1VQbgLPUX3iip6sKwwTxkrj2AGgzxXPV3w4NPeur5MjJvT3T0Tku72lcFiHPyjUuMhlMRzHDKREt58R1ba2uRiBh-G5NkolAK5nu4-41ax93kZLjVk17W2kqYkGLUwoi5GIHLzqeM6uuswNyQRgzpCXEi_-rB9OW5-JY")
        time.sleep(5)


if __name__ == "__main__":
    main()
