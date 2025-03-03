import os

def flowpilot_root():
    return "/data/data/ai.flow.android/files/app"

def internal(path):
    return os.path.join(flowpilot_root(), path)

def external_android_storage():
    return "/storage/emulated/0/flowpilot"

BASEDIR = flowpilot_root()
