GET http://127.0.0.1:8082/config?device_name=ad7606c18&uri=ip:192.168.10.2

HTTP 200
[Asserts]
jsonpath "$.status" == "ok"
jsonpath "$.config.voltage0.calibbias" == "24808"
jsonpath "$.config.voltage0.calibbias_available" count == 3
jsonpath "$.config.voltage0.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage0.calibbias_available[1]" == "4"
jsonpath "$.config.voltage0.calibbias_available[2]" == "508"
jsonpath "$.config.voltage0.convdelay" == "0.006330000"
jsonpath "$.config.voltage0.convdelay_available" count == 3
jsonpath "$.config.voltage0.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage0.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage0.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage0.scale" == "0.076294"
jsonpath "$.config.voltage0.scale_available" count == 5
jsonpath "$.config.voltage0.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage0.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage0.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage0.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage0.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage1.calibbias" == "24808"
jsonpath "$.config.voltage1.calibbias_available" count == 3
jsonpath "$.config.voltage1.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage1.calibbias_available[1]" == "4"
jsonpath "$.config.voltage1.calibbias_available[2]" == "508"
jsonpath "$.config.voltage1.convdelay" == "0.006330000"
jsonpath "$.config.voltage1.convdelay_available" count == 3
jsonpath "$.config.voltage1.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage1.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage1.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage1.scale" == "0.076294"
jsonpath "$.config.voltage1.scale_available" count == 5
jsonpath "$.config.voltage1.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage1.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage1.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage1.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage1.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage2.calibbias" == "24808"
jsonpath "$.config.voltage2.calibbias_available" count == 3
jsonpath "$.config.voltage2.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage2.calibbias_available[1]" == "4"
jsonpath "$.config.voltage2.calibbias_available[2]" == "508"
jsonpath "$.config.voltage2.convdelay" == "0.006330000"
jsonpath "$.config.voltage2.convdelay_available" count == 3
jsonpath "$.config.voltage2.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage2.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage2.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage2.scale" == "0.076294"
jsonpath "$.config.voltage2.scale_available" count == 5
jsonpath "$.config.voltage2.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage2.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage2.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage2.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage2.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage3.calibbias" == "24808"
jsonpath "$.config.voltage3.calibbias_available" count == 3
jsonpath "$.config.voltage3.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage3.calibbias_available[1]" == "4"
jsonpath "$.config.voltage3.calibbias_available[2]" == "508"
jsonpath "$.config.voltage3.convdelay" == "0.006330000"
jsonpath "$.config.voltage3.convdelay_available" count == 3
jsonpath "$.config.voltage3.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage3.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage3.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage3.scale" == "0.076294"
jsonpath "$.config.voltage3.scale_available" count == 5
jsonpath "$.config.voltage3.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage3.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage3.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage3.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage3.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage4.calibbias" == "24808"
jsonpath "$.config.voltage4.calibbias_available" count == 3
jsonpath "$.config.voltage4.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage4.calibbias_available[1]" == "4"
jsonpath "$.config.voltage4.calibbias_available[2]" == "508"
jsonpath "$.config.voltage4.convdelay" == "0.006330000"
jsonpath "$.config.voltage4.convdelay_available" count == 3
jsonpath "$.config.voltage4.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage4.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage4.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage4.scale" == "0.076294"
jsonpath "$.config.voltage4.scale_available" count == 5
jsonpath "$.config.voltage4.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage4.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage4.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage4.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage4.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage5.calibbias" == "24808"
jsonpath "$.config.voltage5.calibbias_available" count == 3
jsonpath "$.config.voltage5.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage5.calibbias_available[1]" == "4"
jsonpath "$.config.voltage5.calibbias_available[2]" == "508"
jsonpath "$.config.voltage5.convdelay" == "0.006330000"
jsonpath "$.config.voltage5.convdelay_available" count == 3
jsonpath "$.config.voltage5.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage5.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage5.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage5.scale" == "0.076294"
jsonpath "$.config.voltage5.scale_available" count == 5
jsonpath "$.config.voltage5.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage5.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage5.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage5.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage5.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage6.calibbias" == "24808"
jsonpath "$.config.voltage6.calibbias_available" count == 3
jsonpath "$.config.voltage6.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage6.calibbias_available[1]" == "4"
jsonpath "$.config.voltage6.calibbias_available[2]" == "508"
jsonpath "$.config.voltage6.convdelay" == "0.006330000"
jsonpath "$.config.voltage6.convdelay_available" count == 3
jsonpath "$.config.voltage6.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage6.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage6.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage6.scale" == "0.076294"
jsonpath "$.config.voltage6.scale_available" count == 5
jsonpath "$.config.voltage6.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage6.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage6.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage6.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage6.scale_available[4]" == "0.095367"
jsonpath "$.config.voltage7.calibbias" == "24808"
jsonpath "$.config.voltage7.calibbias_available" count == 3
jsonpath "$.config.voltage7.calibbias_available[0]" == "-512"
jsonpath "$.config.voltage7.calibbias_available[1]" == "4"
jsonpath "$.config.voltage7.calibbias_available[2]" == "508"
jsonpath "$.config.voltage7.convdelay" == "0.006330000"
jsonpath "$.config.voltage7.convdelay_available" count == 3
jsonpath "$.config.voltage7.convdelay_available[0]" == "0.000000000"
jsonpath "$.config.voltage7.convdelay_available[1]" == "0.000001000"
jsonpath "$.config.voltage7.convdelay_available[2]" == "0.000255000"
jsonpath "$.config.voltage7.scale" == "0.076294"
jsonpath "$.config.voltage7.scale_available" count == 5
jsonpath "$.config.voltage7.scale_available[0]" == "0.019073"
jsonpath "$.config.voltage7.scale_available[1]" == "0.038147"
jsonpath "$.config.voltage7.scale_available[2]" == "0.047684"
jsonpath "$.config.voltage7.scale_available[3]" == "0.076294"
jsonpath "$.config.voltage7.scale_available[4]" == "0.095367"
