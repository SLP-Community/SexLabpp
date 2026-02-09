import shutil
from os import listdir
from os.path import isfile, join

languages = [
  "CHINESE",
  "CZECH",
  "DANISH",
  # "ENGLISH",
  "FINNISH",
  "FRENCH",
  "GERMAN",
  "GREEK",
  "ITALIAN",
  "JAPANESE",
  "NORWEGIAN",
  "POLISH",
  "RUSSIAN",
  "SPANISH",
  "SWEDISH",
  "TURKISH"
]
translated_languages = [
  "RUSSIAN",
]

path = "Interface\\Translations"
f_english = [f for f in listdir(path) if isfile(join(path, f)) and f.endswith("ENGLISH.txt")]

if len(f_english) < 1:
  print("Missing Translation_ENGLISH.txt in directory")
  exit()

f_raw = f_english[0].replace("ENGLISH.txt", "")
en_path = join(path, f_english[0])

def parse_file(file_path):
  with open(file_path, 'r', encoding='utf-16le') as file:
    lines = file.readlines()
  keys = {line.split(maxsplit=1)[0]: line for line in lines if line.startswith('$')}
  return keys, lines

if len(translated_languages) > 0:
  en_keys, en_lines = parse_file(en_path)

def copy_new_keys(file_path):
  l_keys, _ = parse_file(file_path)
  with open(file_path, 'w', encoding='utf-16le') as l_file:
    for line in en_lines:
      if line.startswith('$'):
        key = line.split(maxsplit=1)[0]
        if key in l_keys:
          l_file.write(l_keys[key])
        else:
          l_file.write("# TODO: " + key + "\n")
          l_file.write(en_keys[key])
      else:
        l_file.write(line)

for l in languages:
  new_path = join(path, f_raw + l + ".txt")
  print(f"Processing {new_path}")
  if l in translated_languages:
    print(f"Copying new keys to {l}")
    copy_new_keys(new_path)
    continue
  shutil.copyfile(en_path, new_path)

print("Done")
