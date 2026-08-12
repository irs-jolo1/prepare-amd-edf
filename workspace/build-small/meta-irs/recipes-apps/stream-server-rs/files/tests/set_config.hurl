# with invalid device_name
POST http://127.0.0.1:8082/config?device_name=notexistent&uri=ip:192.168.10.2
Content-Type: application/json
{}
HTTP 303



POST http://127.0.0.1:8082/config?device_name=ad7606c18&uri=ip:192.168.10.2
Content-Type: application/json
{
  "voltage0": {
    "calibbias": "16",
    "convdelay":"0.000002000",
    "scale":"0.038147"
  },
  "voltage1":{
    "calibbias":"-20",
    "convdelay":"0.000006000",
    "scale":"0.076294"
  }
}
HTTP 200
[Asserts]
jsonpath "$.status" == "ok"
jsonpath "$.config.voltage0.calibbias" == "16"
jsonpath "$.config.voltage0.convdelay" == "0.000002000"
jsonpath "$.config.voltage0.scale" == "0.038147"
jsonpath "$.config.voltage1.calibbias" == "-20"
jsonpath "$.config.voltage1.convdelay" == "0.000006000"
jsonpath "$.config.voltage1.scale" == "0.076294"

GET http://127.0.0.1:8082/config?device_name=ad7606c18&uri=ip:192.168.10.2
HTTP 200
[Asserts]
jsonpath "$.status" == "ok"

jsonpath "$.config.voltage0.calibbias" == "16"
jsonpath "$.config.voltage0.convdelay" == "0.000002000"
jsonpath "$.config.voltage0.scale" == "0.038147"

jsonpath "$.config.voltage1.calibbias" == "-20"
jsonpath "$.config.voltage1.convdelay" == "0.000006000"
jsonpath "$.config.voltage1.scale" == "0.076294"
