How to run:

I ran everything here just using visual studio code debugger, and tbh idk how to run it without vscode

u might need to change launch.json and tasks.json (ur paths are probably different), but once they are set u should be able to just click go to the run and debug tab (CTRL + SHIFT + D) and run "Debug FMPartitioning"

NOTE: if u do change launch.json or tasks.json, make sure that u dont push those changes, as that will then break my config. Better yet, once u figure out ur settings remove them from the .gitignore so that they never get pushed. (.gitignore lines 36 and 37)

also make sure that u create a build directory and run make, i think thats necessary. thats also where the executable should end up
