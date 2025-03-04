package ai.flow.modeld.messages;

import ai.flow.definitions.MessageBase;
import ai.flow.definitions.Definitions;
import ai.flow.modeld.ParsedOutputs;

import org.capnproto.PrimitiveList;

import java.nio.ByteBuffer;

public class MsgCameraOdometery extends MessageBase {

    public Definitions.CameraOdometry.Builder odometry;

    public PrimitiveList.Float.Builder rot;
    public PrimitiveList.Float.Builder rotStd;
    public PrimitiveList.Float.Builder trans;
    public PrimitiveList.Float.Builder transStd;
//    public PrimitiveList.Float.Builder wideFromDeviceEuler;
//    public PrimitiveList.Float.Builder wideFromDeviceEulerStd;
//    public PrimitiveList.Float.Builder roadTransformTrans;
//    public PrimitiveList.Float.Builder roadTransformTransStd;

    public MsgCameraOdometery(ByteBuffer rawMessageBuffer) {
        super(rawMessageBuffer);
        initFields();
        bytesSerializedForm = computeSerializedMsgBytes();
        initSerializedBuffer();
    }

    public MsgCameraOdometery() {
        super();
        initFields();
        bytesSerializedForm = computeSerializedMsgBytes();
        initSerializedBuffer();
    }

    private void initFields(){
        event = messageBuilder.initRoot(Definitions.Event.factory);
        odometry = event.initCameraOdometry();

        rot = odometry.initRot(3);
        rotStd = odometry.initRotStd(3);
        trans = odometry.initTrans(3);
        transStd = odometry.initTransStd(3);
//        wideFromDeviceEuler = odometry.initWideFromDeviceEuler(3);
//        wideFromDeviceEulerStd = odometry.initWideFromDeviceEulerStd(3);
//        roadTransformTrans = odometry.initRoadTransformTrans(3);
//        roadTransformTransStd = odometry.initRoadTransformTransStd(3);
    }

    public void fill(ParsedOutputs parsed, long timestamp, int frameId) {
        odometry.setTimestampEof(timestamp);
        odometry.setFrameId(frameId);

        for (int i = 0; i < 3; i++) {
            rot.set(i, parsed.rot[i]);
            rotStd.set(i, parsed.rotStd[i]);
            trans.set(i, parsed.trans[i]);
            transStd.set(i, parsed.transStd[i]);
        }
    }
}
