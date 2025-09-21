import pathlib
exe = pathlib.Path(r"build/Release/cursedarch_tests.exe")
print(exe.exists())
if exe.exists():
    data = exe.read_bytes()
    print('HashTableTest' if b'HashTableTest' in data else 'no-hash')
