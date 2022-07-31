from datetime import datetime
import numpy as np
from elasticsearch import Elasticsearch
import pandas as pd
from keras.engine.input_layer import InputLayer
from keras.layers import Dense, LSTM
import logging
from minio import Minio
import io
import pytz
import os
from tensorflow.keras.layers import *
from tensorflow.keras.models import Sequential
from tensorflow.keras.losses import MeanSquaredError
from tensorflow.keras.metrics import RootMeanSquaredError
from tensorflow.keras.optimizers import Adam

from opentelemetry import trace
from opentelemetry.exporter.jaeger.proto.grpc import JaegerExporter
from opentelemetry.sdk.resources import SERVICE_NAME, Resource
from opentelemetry.sdk.trace import TracerProvider
from opentelemetry.sdk.trace.export import BatchSpanProcessor

logging.basicConfig(level=logging.INFO)

# More info @ https://opentelemetry.io/docs/instrumentation/python/exporters/

# Can name your service anything, please attach group number at the end, i.e., service-name-<groupid>
trace.set_tracer_provider(
    TracerProvider(
        resource=Resource.create({SERVICE_NAME: "service-name-group09"})
    )
)

jaeger_exporter = JaegerExporter(
    collector_endpoint='138.246.234.236:14250',
    insecure=True
)

trace.get_tracer_provider().add_span_processor(
    BatchSpanProcessor(jaeger_exporter)
)

tracer = trace.get_tracer(__name__)

minioAddress = "iot-minio-exposed.default.svc.cluster.local:9900"

# Connecting to MinIO
minioClient = Minio(minioAddress,
                    access_key="group9",
                    secret_key="aEVVai8cjm",
                    secure=False)

logging.info(f"Minioclient: {minioClient}")


def main(request):
    elastic_search_host = "http://group9:aEVVai8cjm@iot-elasticsearch.default.svc.cluster.local:9200"
    client = Elasticsearch(
        elastic_search_host,
    )
    es_query = {"query": {
        "match_all": {}
    }
    }
    # Please attach group-id to spans, e.g. getesdata-group2
    with tracer.start_as_current_span("getestdata-group09"):
        result = client.search(index="11_7_7", body=es_query, size=30000, scroll="5m")
        with tracer.start_as_current_span("parsedata"):
            df = parse_data(result)
            second_df = df.iloc[-1:]
            df = df[~df.index.duplicated(keep='first')]
            temp = df['SensorValues']

            input_data, label = parse_data_to_sets(temp)
            model = add_model()
            with tracer.start_as_current_span("trainmodel"):
                model = train_model(model, input_data, label)
                with tracer.start_as_current_span("savemodel"):
                    save_path = './model.h5'
                    model.save(save_path)

                    with open('./model.h5', 'rb') as file_data:
                        file_stat = os.stat('./model.h5')
                        minioClient.put_object('iot-group-9', 'model.h5', file_data,
                                               file_stat.st_size)

            csv_bytes = second_df.to_csv(index=False).encode('utf-8')
            csv_buffer = io.BytesIO(csv_bytes)

            secondResult = minioClient.put_object(
                bucket_name="iot-group-9",
                object_name="last_value.csv",
                data=csv_buffer,
                length=len(csv_bytes),
                content_type='application/csv'
            )
    return {"result": "done"}


def add_model():
    model = Sequential()
    model.add(InputLayer((3, 1)))
    model.add(LSTM(64))
    model.add(Dense(8, 'relu'))
    model.add(Dense(1, 'linear'))
    model.compile(loss=MeanSquaredError(), optimizer=Adam(learning_rate=0.0001), metrics=[RootMeanSquaredError()])
    return model


def train_model(model, input_data, label):
    model.fit(input_data, label, epochs=10)
    return model


def parse_data_to_sets(df):
    # with WINDOW_SIZE=3, the label is 15min in future (since we have a measurement of every 5min)
    WINDOW_SIZE = 3
    input, label = df_to_X_y(df, WINDOW_SIZE)
    return input, label


# this method ensures that the label is 5min*windows_size in the future
def df_to_X_y(df, window_size):
    df_as_np = df.to_numpy()
    X = []
    y = []
    for i in range(len(df_as_np) - window_size):
        row = [[a] for a in df_as_np[i:i + window_size]]
        X.append(row)
        label = df_as_np[i + window_size]
        y.append(label)
    return np.array(X), np.array(y)


def parse_data(json_data):
    time_values = []
    sensor_values = []
    scroll_id = json_data["_scroll_id"]
    total_data_values = json_data["hits"]["total"]
    logging.info(f'Scroll ID: {scroll_id}')
    logging.info(f'Total Data Values: {total_data_values}')

    data_values = json_data["hits"]["hits"]
    for i in range(0, len(data_values)):
        timestamp = data_values[i]["_source"]["timestamp"]
        tz = pytz.timezone('Europe/Berlin')
        correct_time = datetime.fromtimestamp(timestamp / 1000.0, tz=tz)
        correct_time = correct_time.replace(tzinfo=None)  # delete +2 info
        data_value = float(data_values[i]["_source"]["value"])
        correct_time = pd.to_datetime(correct_time)
        time_values.append(correct_time)
        sensor_values.append(data_value)
    df = pd.DataFrame(
        {
            "TimeStamps": time_values,
            "SensorValues": sensor_values
        }
    )
    df = df.sort_values(by='TimeStamps')

    return df
