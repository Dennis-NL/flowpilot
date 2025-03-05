import os

def flowpilot_root():
    return "/data/data/ai.flow.android/files/app"

def internal(path):
    return os.path.join(flowpilot_root(), path)

def external_android_storage():
    return "/mnt/media_rw/9A49-5E41"

BASEDIR = flowpilot_root()
