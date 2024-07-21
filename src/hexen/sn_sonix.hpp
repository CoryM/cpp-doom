#ifndef __SN_SONIX_HPP__
#define __SN_SONIX_HPP__

void SN_InitSequenceScript(void);
void SN_StartSequence(mobj_t *mobj, int sequence);
void SN_StartSequenceName(mobj_t *mobj, char *name);
void SN_StopAllSequences(void);
void SN_StopSequence(mobj_t *mobj);
void SN_UpdateActiveSequences(void);
int  SN_GetSequenceOffset(int sequence, int *sequencePtr);
void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume, int currentSoundID);

struct seqnode_t {
    int       *sequencePtr;
    int        sequence;
    mobj_t    *mobj;
    int        currentSoundID;
    int        delayTics;
    int        volume;
    int        stopSound;
    seqnode_t *prev;
    seqnode_t *next;
};

extern int        ActiveSequences;
extern seqnode_t *SequenceListHead;


#endif //__SN_SONIX_HPP__