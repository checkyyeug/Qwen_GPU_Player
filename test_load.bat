echo load test.wav > test_load.txt
echo bitrate 128 >> test_load.txt
echo play test.wav >> test_load.txt
.\build\Release\gpu_player.exe < test_load.txt
del test_load.txt