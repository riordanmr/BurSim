@echo off
if %3a == a goto bad
del *~
for %%f in (%3) DO del %%f.cln && txtrep -e -v -s %1 %2 %%f
goto end
:bad
echo Usage: rep "search_str" "rep_str" filemask
:end
