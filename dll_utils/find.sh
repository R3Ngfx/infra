for file in $(ldd bin/infra.exe | grep mingw64 | python dll_utils/parse.py)
do
    echo "Getting DLL $file..."
    cp $file bin/$(basename $file)
done
