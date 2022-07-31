import datetime

from dateutil import parser
import time

import pandas as pd
import tensorflow as tf
import numpy as np
import paho.mqtt.client as mqtt
import requests
from minio import Minio


def invoke_openwhisk():
    url = "https://138.246.236.111:31001/api/v1/web/guest/default/func-11-action-function-141"
    requests.get(url, verify=False)


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
    time_last_value = last_value['TimeStamps'].iloc[0]  # string
    lst = []
    lst1 = [value]
    lst.append(lst1)
    value_list = np.array(lst)
    prediction = model.predict(value_list).round(decimals=0)
    date_time_obj = parser.parse(time_last_value)
    date_time_obj = date_time_obj + datetime.timedelta(minutes=15)
    predict_time = date_time_obj.timestamp()
    predict_time = int(predict_time) * 1000
    prediction = int(prediction[0][0])
    return prediction, predict_time


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
        msg = "{{\"username\":\"group9\",\"device_id\":\"7\",\"prediction\":\"{}\",\"timestamp\":\"{}\"}}".format(
            prediction, predict_time)
    client.publish(topic, msg)


def main():
    # correct run:
    while True:
        invoke_openwhisk()
        prediction, predict_time = get_prediction()
        JWT_TOKEN = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2NTc5MjgzNDEsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMTFfNyJ9.MKVv1Sb4nNF1MpWK26uvueyh1vuCK45AttPvcLtWtkbTb3Hh6yV05tHDgIzjFly1T0FptSCH1JKDeti2HOXGSygDtK_5LW8al9mEMsRaMWpoabq2rsw9LnUACKtZ8nBEuxhyLP5ikIPxgDqW5rWWfWOuXJGK3WxERnL7NOMyfj_-Jl0HDqCwcnagbW-HWlkVmfxLzc4gLm4TUbXFGj9VHxaHR5akI93Z8shnsbKqcgO1ZGOR56-YhQqj-7eQAtBzpAYRANVUG3PK14D6Vfnv2qKVsk_No6dSPcbiBNnWRc1RODjlX097KE4n9idwiaTC2tUf9x21IOdmwOfVUHDY34FyLXJQBcaF-usf0ZWNi2uWk6OQVN7P91lN_0cK0ZXLe6wGkMF_ZdMYF5OTsJBW2XgsZgbGyzuznx4XkSmBtOjg2fxQ-zm0_J3EoElZH1ocmJppyYOUMFyKKKckqWhnmtpVmNngzizgXLuKfPaSRtaEDF_c7GLJ0RNfUPD6JN0GxlsMqBPVHPBMSv5RcY9ylNCjDPNR1hjRcwApoScGtrW7r-TDpALqFH8fM0D0oVTAA5OEcVu6cV83aj0xvSs-K7amOlgAJSOyIryJ0Y0Sn3Q9a1iHZkzjUupkIiZpe599-fy-CcCVuhZYD3na53jBy_lyCdVwUO79YjHIEd3C2rs"
        send_to_broker(prediction, predict_time, 'broker.emqx.io', f"/iotLabPrediction/{predict_time}")
        send_to_broker(prediction, predict_time, '138.246.236.107', "11_7", JWT_TOKEN)
        time.sleep(120)


if __name__ == "__main__":
    main()
