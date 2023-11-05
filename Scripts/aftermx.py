import os
import subprocess

# subprocess.run(["git", "restore", "CMakeLists.txt"])
os.rename("../Core/Src/main.c", "../Core/Src/main.cpp")
