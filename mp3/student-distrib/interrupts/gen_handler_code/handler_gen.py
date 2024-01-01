handlers = []
with open("handlers.txt", "r") as f:
    for line in f:
        handlers.append(line.split()[2])

task = []
for handler in handlers:
    task.append('_'.join(handler.split('_')[:-1]))

with open("ccode.c", "w") as f:
    for h, t in zip(handlers, task):
        s = f"void {h} ()" + "{\n\tPRINT_HANDLER(" + f"\"{t}\"" + ");\n\twhile(1);\n" + "}\n\n"
        f.write(s)
