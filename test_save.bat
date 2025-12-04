echo play test.wav > temp_commands.txt
echo bitrate 128 >> temp_commands.txt
echo save output.wav >> temp_commands.txt
.\build\Release\gpu_player.exe < temp_commands.txt
del temp_commands.txt