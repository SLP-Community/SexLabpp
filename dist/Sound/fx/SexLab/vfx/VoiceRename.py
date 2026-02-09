import os

def rename_files_in_directory(directory):
  for profile_name in os.listdir(directory):
    profile_path = os.path.join(directory, profile_name)
    if os.path.isdir(profile_path):
      for subfolder_name in os.listdir(profile_path):
        subfolder_path = os.path.join(profile_path, subfolder_name)
        if os.path.isdir(subfolder_path):
          files = [f for f in os.listdir(subfolder_path) if f.endswith('.mp3') or f.endswith('.wav')]
          for index, file_name in enumerate(files, start=1):
            file_extension = file_name.split('.')[-1]
            new_file_name = f"{profile_name}_{subfolder_name[0]}{index:02d}.{file_extension}"
            old_file_path = os.path.join(subfolder_path, file_name)
            new_file_path = os.path.join(subfolder_path, new_file_name)
            os.rename(old_file_path, new_file_path)
            print(f"Renamed: {old_file_path} to {new_file_path}")

if __name__ == "__main__":
  script_directory = os.path.dirname(os.path.abspath(__file__))
  rename_files_in_directory(script_directory)