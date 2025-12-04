@echo off
echo play test.wav > input.txt
.\build\Release\gpu_player.exe < input.txt
del input.txt