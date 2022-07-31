from datetime import datetime
# from elasticsearch6 import Elasticsearch
from elasticsearch import Elasticsearch
import pandas as pd
import time
from scipy.stats import norm
import statsmodels.api as sm
# from statsmodels.iolib.smpickle import save_pickle, load_pickle
import pickle
import pytz
import logging
from minio import Minio
import io
from datetime import datetime


logging.basicConfig(level=logging.INFO)

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
    result = client.search(index="11_7_7", body=es_query, size=10000, scroll="5m")
    df = parse_data(result)
    logging.info(f'{result}')
    # model = test_model(df)
    # # save_pickle(model, "/tmp/test_model.pkl")
    # bytes_file = pickle.dumps(model)
    #
    # Connecting to MinIO
    buckets = minioClient.list_buckets()
    for bucket in buckets:
        print(bucket.name, bucket.creation_date)
    logging.info(f"BucketName: {bucket.name}, CreationDate: {bucket.creation_date}")

    # Storing Model

    csv_bytes = df.to_csv(index=False).encode('utf-8')
    csv_buffer = io.BytesIO(csv_bytes)

    result = minioClient.put_object(
        bucket_name="iot-group-9",
        object_name="new_data.csv",
        data=csv_buffer,
        # data=io.BytesIO(df),
        length=len(csv_bytes),
        content_type='application/csv'
    )

    print(
        "created {0} object; etag: {1}, version-id: {2}".format(
            result.object_name, result.etag, result.version_id,
        ),
    )

    return {"result": "done"}


def test_model(df):
    mod = sm.tsa.statespace.SARIMAX(df['SensorValues'], trend='c', order=(1, 1, (1, 0, 0, 1)))
    model = mod.fit(disp=False)
    # logging.info(f'{model.summary()}')
    return model


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
        correct_time = datetime.fromtimestamp(timestamp/1000.0, tz=tz)
        correct_time = correct_time.replace(tzinfo=None)  # delete +2 info
        data_value = float(data_values[i]["_source"]["value"])
        # timestamp_converted = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp/1000.0))
       # logging.info(f'TimeStamp: {timestamp}')
        #logging.info(f'TimeStamp Converted: {correct_time}')
        #logging.info(f'Value: {data_value}')
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
