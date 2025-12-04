@echo off
echo play music.flac > temp_input.txt
.\build\Release\gpu_player.exe < temp_input.txt
del temp_input.txt