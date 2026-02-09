import os
import subprocess

def convert_mp3_to_wav(directory):
  for root, _, files in os.walk(directory):
    for file in files:
      if file.endswith(".mp3"):
        mp3_path = os.path.join(root, file)
        wav_path = os.path.splitext(mp3_path)[0] + ".wav"
        if not os.path.exists(wav_path):
          subprocess.run(["ffmpeg", "-i", mp3_path, wav_path])
        os.remove(mp3_path)

if __name__ == "__main__":
  directory = os.path.dirname(os.path.abspath(__file__))
  convert_mp3_to_wav(directory)