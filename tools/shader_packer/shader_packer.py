import os

HEADER = """\
#pragma once
/*
AUTO GENERATED DO NOT EDIT
*/

"""


def main():
    directory = "./shaders"
        
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        if filename.endswith(".fs") or filename.endswith(".vs"): 
            filepath = os.path.join(directory, filename)
            # print(filepath)
            print(f"Packing {filename}")

            with open(filepath, "r") as file:
                file_text = file.read()
                
                bytes = file_text.encode()
                [name, extension] = filename.split(".")
                with open(f"src/gen/{filename}.h", "w") as output:
                    output.write(HEADER)
                    output.write("/*\n")
                    output.write(file_text)
                    output.write("\n*/\n")
                    output.write("static const char* {name}_{extension} = \n".format(name=name, extension=extension))
                    for line in file_text.splitlines(False):
                        output.write("\t\"")
                        output.write(line)
                        output.write("\\n\"\n")
                    output.write(";")

            continue
        else:
            continue
        pass
    print("Done!")








if __name__ == "__main__":
    main()
