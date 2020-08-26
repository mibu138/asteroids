#ifndef R_PIPELINE_H
#define R_PIPELINE_H

#include "v_def.h"

extern VkPipeline pipelines[MAX_PIPELINES];

enum {R_OBJ_PIPELINE, R_EMIT_PIPELINE, R_POST_PROC_PIPELINE};

void initDescriptorSets(void);
void initPipelines(void);

void cleanUpPipelines(void);

#endif /* end of include guard: R_PIPELINE_H */

