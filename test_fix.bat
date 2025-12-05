echo load test.wav > input.txt
echo play test.wav >> input.txt
echo pause >> input.txt
echo stop >> input.txt
.\build\Release\gpu_player.exe < input.txt
del input.txt