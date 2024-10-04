Import("env")
print("############### replace mklittlefs start")
env.Replace( MKSPIFFSTOOL= "./mklittlefs.exe" )
print("############### replace mklittlefs end")



