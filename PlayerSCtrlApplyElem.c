/**
 * @file PlayerSCtrlApplyElem.c
 * @author Halkaze
 * @brief PlayerSCtrlApplyElem（ステコン処理）関連関数群
 * @date 2023-03-17
 * @copyright Copyright (c) 2023
 *
 * 留意事項
 * ・ZeroMemoryはすべてSecureZeroMemoryに置換してあります
 * ・コンパイル結果がwinmugenのものとまったく同一となることは保証できません
 * ・コンパイル時最適化の結果, EvalExpression系関数の
 *   実行タイミングや評価回数が変わる可能性があります
 *
 * "R_"接頭辞付き関数は，バイナリから再現した関数
 *
 * "static inline"付き関数およびマクロ関数は，バイナリでは
 * インライン化されているが，便宜のために独自に定義した関数
 *
 * 以下にてコンパイル確認済み
 * ・gcc (Rev10, Built by MSYS2 project) 12.2.0
 * ・clang version 15.0.7 (i686-w64-windows-gnu)
 *
 */

#include "mugen.h"

#include <stdarg.h>
#include <stdbool.h>

uint32_t R_PlayerDoSCtrlApplyElem(MUGEN_PLAYER* player, MUGEN_SC_DATA* scData, MUGEN_PLAYER_REDIRECTS* redirects) __attribute__((nonnull(1, 2, 3)));
uint32_t R_PlayerSCtrlApplyElem(MUGEN_PLAYER* player, MUGEN_SC_DATA* scData, MUGEN_PLAYER_REDIRECTS* redirects) __attribute__((nonnull(1, 2, 3)));

uint32_t R_SCtrlInitHitOverride(MUGEN_SC_PARAMS_EX_HITOVERRIDE* scParamsEx, MUGEN_HIT_OVERRIDE* hitOverride, MUGEN_PLAYER* player) __attribute__((nonnull(1, 2, 3)));
uint32_t R_SCtrlInitEnvShake(MUGEN_SC_PARAMS_EX_ENVSHAKE* paramsEx, MUGEN_ENV_SHAKE* envShake, MUGEN_PLAYER* player) __attribute__((nonnull(1, 2, 3)));
uint32_t R_SCtrlInitAftImgParams(MUGEN_SC_PARAMS_EX_AFTERIMAGE* paramsEx, MUGEN_AFTERIMAGE_DATA* afterImage, MUGEN_ANIM_USED* anim, MUGEN_PLAYER* player) __attribute__((nonnull(1, 2, 3, 4)));
void R_AftImgSetup(MUGEN_AFTERIMAGE_DATA* afterImage, int32_t data[static 24], MUGEN_ANIM_USED* anim) __attribute__((nonnull(1, 2, 3)));
uint32_t R_SCtrlInitPause(MUGEN_SC_PARAMS_EX_PAUSE* paramsEx, uint32_t pauseType, MUGEN_PLAYER* player) __attribute__((nonnull(1, 3)));
uint32_t R_SCtrlInitPalFX(MUGEN_SC_PARAMS_EX_PALFX* paramsEx, MUGEN_PAL_FX* palFX, MUGEN_PLAYER* player) __attribute__((nonnull(1, 2, 3)));
void R_ExplodMakeFromInfo(MUGEN_PLAYER* player, MUGEN_SC_PARAMS_EX_EXPLOD* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects) __attribute__((nonnull(1, 2, 3)));
void R_ExplodDefault(MUGEN_EXPLODS_DATA* explod) __attribute__((nonnull(1)));
void R_ExplodSetParams(MUGEN_PLAYER* player, MUGEN_EXPLODS_DATA* explod, MUGEN_SC_PARAMS_EX_EXPLOD* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects, uint32_t isUpdate) __attribute__((nonnull(1, 2, 3, 4)));
void R_ExplodBindChange(MUGEN_EXPLODS_INFO* explods, int32_t playerId, int32_t explodId, int32_t bindTime) __attribute__((nonnull(1)));
void R_ExplodModifyFromInfo(MUGEN_PLAYER* player, MUGEN_SC_PARAMS_EX_EXPLOD* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects) __attribute__((nonnull(1, 2, 3)));
MUGEN_PLAYER* R_HelperMake(MUGEN_PLAYER* parent, MUGEN_SC_PARAMS_EX_HELPER* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects) __attribute__((nonnull(1, 2, 3)));
void R_PlayerReset(MUGEN_PLAYER* player);
void R_HelperDestroy(MUGEN_PLAYER* player) __attribute__((nonnull(1)));
void R_SCtrlDoPlaySndInfo(MUGEN_PLAYER* player, MUGEN_SC_PARAMS_EX_PLAYSND* paramsEx) __attribute__((nonnull(1)));
uint32_t R_SCtrlInitHitDef(MUGEN_SC_PARAMS_EX_HITDEF* paramsEx, MUGEN_HIT_DEF_PARAMS* hitDefParams, MUGEN_PLAYER* player) __attribute__((nonnull(1, 2, 3)));
int32_t R_SCtrlInitProjectile(MUGEN_SC_PARAMS_EX_PROJECTILE* paramsEx, MUGEN_PLAYER* player, MUGEN_PLAYER* enemyFirst, MUGEN_PLAYER* owner) __attribute__((nonnull(1, 2, 4)));

static inline void SCtrlClipboard(MUGEN_PLAYER* player, MUGEN_SC_ID scId, int32_t argc, char* text, MUGEN_EVAL_VALUE* params) __attribute__((nonnull(1, 4, 5)));

uint32_t R_PlayerDoSCtrlApplyElem(MUGEN_PLAYER* player, MUGEN_SC_DATA* scData, MUGEN_PLAYER_REDIRECTS* redirects) {
  MUGEN_STATE_DEF_INFO** preEnforcedStateList = player->enforcedStateList;
  (player->ai).preStateNo = player->stateNo;
  (player->ai).preStateType = player->stateType;
  (player->ai).preMoveType = player->moveType;
  (player->ai).preCtrl = player->ctrl;
  if (PlayerSCtrlApplyElem(player, scData, redirects) != 0) {
    if (preEnforcedStateList == NULL && player->enforcedStateList == NULL) {
      int32_t* history = (player->ai).commandHistory;
      if (AICountCommands(history) > 0) {
        AICopyCommands((player->ai).commands, history);
        (player->ai).gametime = g->gameTime;
        PlayerAILearnMoveStart(player);
        PlayerAIMsgAttack(player);
      }
    }
    return 1;
  }
  return 0;
}

uint32_t R_PlayerSCtrlApplyElem(MUGEN_PLAYER* player, MUGEN_SC_DATA* scData, MUGEN_PLAYER_REDIRECTS* redirects) {
  switch (scData->scID) {
    case ID_SC_HitOverride: {
      int32_t slot = EvalExpressionI(player, &(scData->params[0]), 1);
      if (slot < 0 || slot > 7) {
        slot = 0;
        Warn(player, "illegal override slot: 0 assumed");
      }
      SCtrlInitHitOverride((scData->paramsEx).hitOverride, &(player->hitOverrides[slot]), player);
    }
      return 0;

    case ID_SC_MoveHitReset:
      player->moveContactType = MOVECONTACT_NONE;
      return 0;

    case ID_SC_AssertSpecial: {
      // $0 = flag
      // $1 = flag2
      // $2 = flag3
      // $3 = flagの種類（0: 全体に作用, Otherwise: 実行者にのみ作用）
      // $4 = flag2の種類（同上）
      // $5 = flag3の種類（同上）
      int32_t flag1 = EvalExpressionI(player, &(scData->params[0]), 0);
      if (flag1 != -1) {
        int32_t flag1Type = EvalExpressionI(player, &(scData->params[3]), 0);
        if (flag1Type == 0) {
          *(uint8_t*)((int32_t)(&(g->assertSpecialFlags)) + flag1) = 1;
        } else {
          *(uint8_t*)((int32_t)(&(player->assertSpecialFlags)) + flag1) = 1;
        }
      }
      int32_t flag2 = EvalExpressionI(player, &(scData->params[1]), 0);
      if (flag2 != -1) {
        int32_t flag2Type = EvalExpressionI(player, &(scData->params[4]), 0);
        if (flag2Type == 0) {
          *(uint8_t*)((int32_t)(&(g->assertSpecialFlags)) + flag2) = 1;
        } else {
          *(uint8_t*)((int32_t)(&(player->assertSpecialFlags)) + flag2) = 1;
        }
      }
      int32_t flag3 = EvalExpressionI(player, &(scData->params[2]), 0);
      if (flag3 != -1) {
        int32_t flag3Type = EvalExpressionI(player, &(scData->params[5]), 0);
        if (flag3Type == 0) {
          *(uint8_t*)((int32_t)(&(g->assertSpecialFlags)) + flag3) = 1;
        } else {
          *(uint8_t*)((int32_t)(&(player->assertSpecialFlags)) + flag3) = 1;
        }
      }
    }
      return 0;

    case ID_SC_EnvShake:
      SCtrlInitEnvShake((scData->paramsEx).envShake, (MUGEN_ENV_SHAKE*)(&(g->envShake)), player);
      return 0;

    case ID_SC_FallEnvShake:
      if ((player->getHitVars).fall_envShake.time >= 1) {
        memcpy((void*)(&(g->envShake)), &((player->getHitVars).fall_envShake), sizeof(MUGEN_ENV_SHAKE));
        (player->getHitVars).fall_envShake.time = 0;
      }
      return 0;

    case ID_SC_EnvColor: {
      // $1 = time
      // $2 = under
      // $3 = value red
      // $4 = value green
      // $5 = value blue
      int32_t red = EvalExpressionI(player, &(scData->params[3]), 1);
      int32_t green = EvalExpressionI(player, &(scData->params[4]), 1);
      int32_t blue = EvalExpressionI(player, &(scData->params[5]), 1);
      g->envColor_value = makecol(red, green, blue);
      g->envColor_time = EvalExpressionI(player, &(scData->params[1]), 1);
      if (g->envColor_time != -1) {
        ++(g->envColor_time);
      }
      if (g->envColor_time < -1) {
        Warn(player, "set illegal envcolortime");
        g->envColor_time = 1;
      }
      g->envColor_under = (EvalExpressionI(player, &(scData->params[2]), 1) != 0);
    }
      return 0;

    case ID_SC_GameMakeAnim: {
      // $0 = value
      // $1 = under
      // $2 = random
      // $3 = pos X
      // $4 = pos Y
      PlayerCalcDraw(player);
      int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
      if (value < 0) {
        value = 0;
        Warn(player, "made negative gameanim");
      }
      int32_t posX = (int32_t)(EvalExpressionF(player, &(scData->params[3])) * player->facing + g->iStageAxisX + player->realScreenPosX);
      int32_t posY = (int32_t)(EvalExpressionF(player, &(scData->params[4])) + g->iStageAxisY + player->realScreenPosY);
      int32_t random = EvalExpressionI(player, &(scData->params[2]), 1);
      if (random < 1) {
        if (random < 0) {
          Warn(player, "negative random param for GameMakeAnim");
        }
      } else {
        uint32_t temp = (uint32_t)rand();
        posX += ((temp % random) - (random >> 1)) * player->facing;
        temp = (uint32_t)rand();
        posY += ((temp % random) - (random >> 1));
      }
      int32_t id = AnimPerArrayAdd(g->persistentAnimArray, g->fightfx_anim, g->fightfx_sff, 0, value, posX, posY, (player->facing != 1), -1);
      int32_t under = EvalExpressionI(player, &(scData->params[1]), 0);
      if (id != -1) {
        AnimPerArraySetNo(g->persistentAnimArray, id, (under == 0 ? 1 : 2));
      }
    }
      return 0;

    case ID_SC_AfterImage:
      if (player->afterImage == NULL) {
        player->afterImage = RequestNewAftImg(g->afterImages);
      }
      if (player->afterImage != NULL) {
        SCtrlInitAftImgParams((scData->paramsEx).afterImage, player->afterImage, player->anim, player);
      }
      return 0;

    case ID_SC_AfterImageTime:
      // $0 = time
      if (player->afterImage != NULL) {
        player->afterImage->elapsedTime = 0;
        player->afterImage->time = EvalExpressionI(player, &(scData->params[0]), 1);
        if (player->afterImage->time < -1) {
          Warn(player, "set illegal afterimagetime");
          player->afterImage->time = 1;
        }
      }
      return 0;

    case ID_SC_Pause:
      SCtrlInitPause((scData->paramsEx).pause, 0, player);
      return 0;

    case ID_SC_SuperPause:
      SCtrlInitPause((scData->paramsEx).pause, 1, player);
      {
        MUGEN_SC_PARAMS_EX_SUPERPAUSE* paramsExSP = (scData->paramsEx).superPause;
        if (EvalExpressionI(player, &(paramsExSP->darken), 1) == 0) {
          (g->sc_superPause).darken = 0;
        } else {
          MUGEN_PAL_GROUP temp_grp1;
          MUGEN_PAL_GROUP temp_grp2;
          MUGEN_PAL_GROUP temp_grp3;
          MUGEN_PAL_GROUP temp_grp4;
          PalGrpMakeFlag(&temp_grp1, 0xffffffff, 1);
          PalGrpMakeFlag(&temp_grp2, 0xffffffff, 3);
          PalGrpOrFlag(&temp_grp3, temp_grp1, temp_grp2);
          PalGrpXorFlag(&temp_grp4, temp_grp3, temp_grp2);
          memcpy((void*)(&((g->sc_superPause).palGrp)), &temp_grp4, sizeof(MUGEN_PAL_GROUP));
          (g->sc_superPause).darken = 1;
        }
        if (player->superMoveTime < 0 || (g->sc_superPause).time < player->superMoveTime) {
          // SCtrlInitPause内で既に範囲チェック + 修正がされているため,
          // この警告文は隔離未満では見られないはず
          Warn(player, "set illegal supermovetime");
          player->superMoveTime = 0;
        }
        int32_t sparkNo = EvalExpressionI(player, &(paramsExSP->anim), 1);
        if (sparkNo < -1) {
          Warn(player, "negative spark animno ignored");
        } else if (sparkNo != -1) {
          int32_t posX = (int32_t)((EvalExpressionF(player, &(paramsExSP->posX)) - player->offsetX) * player->facing * player->scale + g->iStageAxisX + player->realScreenPosX);
          int32_t posY = (int32_t)((EvalExpressionF(player, &(paramsExSP->posY)) - player->offsetY) * player->scale + g->iStageAxisY + player->realScreenPosY);
          PlayerSparkCreate(player, sparkNo, posX, posY, (paramsExSP->animFlag == 0), (player->facing != 1), 0);
        }
        float p2DefMul = EvalExpressionF(player, &(paramsExSP->p2DefMul));
        if (p2DefMul <= 0.0F) {
          if (p2DefMul != 0.0F) {
            Warn(player, "negative p2defmul for SuperPause");
          }
          // p2DefMul = 0.0F の場合も修正される
          p2DefMul = (g->rules).super_targetDefenceMul;
        }
        if (p2DefMul != 1.0F) {
          MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
          while (data != NULL) {
            MUGEN_PLAYER* p = data->player;
            if (p->isGuarding == 0) {
              p->defenceMul = (p->defenceMul) / p2DefMul;
            }
            data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
          }
        }
        int32_t powerAdd = EvalExpressionI(player, &(paramsExSP->powerAdd), 1);
        PlayerPowerSet(redirects->teamFirst, *((redirects->teamFirst)->pPower) + powerAdd);
        int32_t unhittable = EvalExpressionI(player, &(paramsExSP->unhittable), 1);
        if (unhittable != 0 && unhittable < (g->sc_superPause).time) {
          player->unhittable = unhittable;
        }
        int32_t soundGrpNo = EvalExpressionI(player, &(paramsExSP->soundGrpNo), 1);
        int32_t soundItemNo = EvalExpressionI(player, &(paramsExSP->soundItemNo), 1);
        MUGEN_SND_INFO** snd;
        if (paramsExSP->soundFlag == 0) {
          snd = player->info->snd;
        } else {
          snd = g->common_snd;
        }
        GameSndPlayIDPos(-1, (int32_t)(player->realScreenPosX), snd, soundGrpNo, soundItemNo);
      }
      return 0;

    case ID_SC_MakeDust: {
      // $0 = pos X
      // $1 = pos Y
      // $2 = spacing
      // $3 = pos2 X
      // $4 = pos2 Y
      // $5 = pos2存在フラグ？
      int32_t spacing = EvalExpressionI(player, &(scData->params[2]), 1);
      if (spacing < 1) {
        Warn(player, "makedust spacing <= 0");
        spacing = 1;
      }
      if ((g->currentStateElapsedTime) % spacing == 0) {
        PlayerCalcDraw(player);
        int32_t temp = EvalExpressionI(player, &(scData->params[0]), 1);
        int32_t posX = (int32_t)((float)((uint32_t)(rand()) % 3) + (float)(temp * player->facing) + g->iStageAxisX + player->realScreenPosX - 1.0F);
        temp = EvalExpressionI(player, &(scData->params[1]), 1);
        int32_t posY = (int32_t)((float)((uint32_t)(rand()) % 3) + (float)(temp) + g->iStageAxisY + player->realScreenPosY - 1.0F);
        int32_t idx = ExplodAdd(g->explods, 120, g->fightfx_anim, g->fightfx_sff);
        if (idx != -1) {
          MUGEN_EXPLODS_DATA* data = &((g->explods->data)[idx]);
          data->axisX = posX;
          data->axisY = posY;
          data->_unknown_1 = 1;
          data->removeTime = AnimGetLooptime(data->anim);
          (data->anim)->sprPriority = 10;
          (data->anim)->spritesLayer = g->prioritySpritesLayer1;
        }
        float ftemp = EvalExpressionF(player, &(scData->params[5]));
        if (ftemp != 0.0F) {
          ftemp = EvalExpressionF(player, &(scData->params[3]));
          int32_t pos2X = (int32_t)((float)((uint32_t)(rand()) % 3) + (float)(ftemp * player->facing) + g->iStageAxisX + player->realScreenPosX - 1.0F);
          ftemp = EvalExpressionF(player, &(scData->params[4]));
          int32_t pos2Y = (int32_t)((float)((uint32_t)(rand()) % 3) + (float)(ftemp) + g->iStageAxisY + player->realScreenPosY - 1.0F);
          idx = ExplodAdd(g->explods, 120, g->fightfx_anim, g->fightfx_sff);
          if (idx != -1) {
            MUGEN_EXPLODS_DATA* data = &((g->explods->data)[idx]);
            data->axisX = pos2X;
            data->axisY = pos2Y;
            data->_unknown_1 = 1;
            data->removeTime = AnimGetLooptime(data->anim);
            (data->anim)->sprPriority = 10;
            (data->anim)->spritesLayer = g->prioritySpritesLayer1;
          }
        }
      }
    }
      return 0;

    case ID_SC_AllPalFx:  // BGPalFX, PalFX も兼用
      SCtrlInitPalFX((scData->paramsEx).palFX, &(player->palFX), player);
      return 0;

    case ID_SC_Explod:
      ExplodMakeFromInfo(player, (scData->paramsEx).explod, redirects);
      return 0;

    case ID_SC_RemoveExplod: {
      // $0 = id
      int32_t id = EvalExpressionI(player, &(scData->params[0]), 1);
      if (id < -1) {
        id = -1;
        Warn(player, "removal of illegal explod ID");
      }
      ExplodRemoveID(g->explods, player->playerId, id);
    }
      return 0;

    case ID_SC_ExplodBindTime: {
      // $0 = id
      // $1 = bindTime
      int32_t id = EvalExpressionI(player, &(scData->params[0]), 1);
      int32_t bindTime = EvalExpressionI(player, &(scData->params[1]), 1);
      if (id < -1) {
        id = -1;
        Warn(player, "illegal ID to explodbindtime");
      }
      if (bindTime < -1) {
        bindTime = -1;
        Warn(player, "illegal explodbindtime");
      }
      ExplodBindChange(g->explods, player->playerId, id, bindTime);
    }
      return 0;

    case ID_SC_ModifyExplod:
      ExplodModifyFromInfo(player, (scData->paramsEx).explod, redirects);
      return 0;

    case ID_SC_Helper:
      HelperMake(player, (scData->paramsEx).helper, redirects);
      return 0;

    case ID_SC_DestroySelf:
      HelperDestroy(player);
      return 0;

    case ID_SC_ParentVarSet:
      // $0 = value
      // $1 = index
      // $2 = type (0: var, 1: fvar)
      if (player->isHelper == 1) {
        if (redirects->parent == NULL) {
          Warn(player, "no parent for parentvarset");
        } else {
          uint32_t type = (uint32_t)(EvalExpressionI(player, &(scData->params[2]), 0));
          int32_t index = EvalExpressionI(player, &(scData->params[1]), 1);
          if (index >= 0 && index <= (type == 0 ? 59 : 39)) {
            if (type == 0) {
              (redirects->parent)->var[index] = EvalExpressionI(player, &(scData->params[0]), 1);
            } else {
              (redirects->parent)->fvar[index] = EvalExpressionF(player, &(scData->params[0]));
            }
          } else {
            Warn(player, "set parentvar out of range");
          }
        }
      }
      return 0;

    case ID_SC_ParentVarAdd:
      // $0 = value
      // $1 = index
      // $2 = type (0: var, 1: fvar)
      if (player->isHelper == 1) {
        if (redirects->parent == NULL) {
          Warn(player, "no parent for parentvaradd");
        } else {
          uint32_t type = (uint32_t)(EvalExpressionI(player, &(scData->params[2]), 0));
          int32_t index = EvalExpressionI(player, &(scData->params[1]), 1);
          if (index >= 0 && index <= (type == 0 ? 59 : 39)) {
            if (type == 0) {
              (redirects->parent)->var[index] += EvalExpressionI(player, &(scData->params[0]), 1);
            } else {
              (redirects->parent)->fvar[index] += EvalExpressionF(player, &(scData->params[0]));
            }
          } else {
            Warn(player, "set parentvar out of range");
          }
        }
      }
      return 0;

    case ID_SC_TagIn:
      // $0 = stateNo
      // $1 = partnerStateNo
      if (redirects->partner != NULL) {
        int32_t stateNo = EvalExpressionI(player, &(scData->params[0]), 1);
        if (stateNo > -1) {
          PlayerChangeState(player, stateNo);
        }
        int32_t partnerStateNo = EvalExpressionI(player, &(scData->params[1]), 1);
        if ((redirects->partner)->facing != player->facing) {
          PlayerTurnAround(redirects->partner);
        }
        if (partnerStateNo > -1) {
          PlayerChangeState(redirects->partner, partnerStateNo);
          (redirects->partner)->standBy = 0;
        } else {
          Warn(player, "attempted change to negative anim");
          return 0;
        }
      }
      return 1;

    case ID_SC_TagOut:
      player->standBy = 1;
      return 1;

    case ID_SC_DisplayToClipboard:
    case ID_SC_AppendToClipboard:
      // $0 = paramsの1つ目
      // $1 = paramsの2つ目
      // $2 = paramsの3つ目
      // $3 = paramsの4つ目
      // $4 = paramsの5つ目
      // $5 = paramsの個数
      // Clipboardの処理は実際にはインライン化されているが, あまりに複雑なため関数に分離させた
      SCtrlClipboard(player, scData->scID, (scData->params)[5].value.i, (char*)((scData->paramsEx).clipboard), scData->params);
      return 0;

    case ID_SC_ClearClipboard:
      ClipboardClear(player->clipboard);
      return 0;

    case ID_SC_BindToParent:
    case ID_SC_BindToRoot:
      // $0 = time
      // $1 = facing
      // $2 = pos X
      // $3 = pos Y
      // $4 = pos Z
      if (scData->scID == ID_SC_BindToParent) {
        if (redirects->parent == NULL) {
          Warn(player, "No parent for bindtoparent");
          return 0;
        }
      } else if (redirects->root == NULL) {
        Warn(player, "No root for bindtoroot");
        return 0;
      }
      int32_t time = EvalExpressionI(player, &(scData->params[0]), 1);
      if (time < -1) {
        time = 1;
        Warn(player, "illegal helper bindtime");
      }
      (player->bindTo).time = time;
      int32_t facing = EvalExpressionI(player, &(scData->params[1]), 1);
      if (facing < -1 || facing > 1) {
        facing = (facing >= 0 ? 1 : -1);
        Warn(player, "bind's facing param not +/-1 or 0");
      }
      (player->bindTo).facing = facing;
      (player->bindTo).posX = EvalExpressionF(player, &(scData->params[3]));
      (player->bindTo).posY = EvalExpressionF(player, &(scData->params[4]));
      (player->bindTo).posZ = EvalExpressionF(player, &(scData->params[5]));
      MUGEN_PLAYER* bindPlayer = ((scData->scID) == ID_SC_BindToParent ? (redirects->parent) : (redirects->root));
      (player->bindTo).player = bindPlayer;
      (player->bindTo).targetScreenPosX = &(bindPlayer->screenPosX);
      (player->bindTo).targetPosY = &(bindPlayer->posY);
      (player->bindTo).targetPosZ = &(bindPlayer->posZ);
      (player->bindTo).targetFacing = &(bindPlayer->facing);
      return 0;

    case ID_SC_Null:
      return 0;

    case ID_SC_ChangeState: {
      // $0 = stateNo
      // $1 = ctrl
      // $2 = anim
      int32_t stateNo = EvalExpressionI(player, &(scData->params[0]), 1);
      if (stateNo < 0) {
        stateNo = 0;
        Warn(player, "attempted change to negative state");
      }
      PlayerChangeState(player, stateNo);
      if (player->isHelper == 0 && player->ctrl != 0 && (player->stateType == STATETYPE_S || player->stateType == STATETYPE_C) && g->autoTurn != 0 && GameModeInPassiveFightScreen() == 0 && (redirects->enemyFirst) != NULL && (redirects->enemyFirst)->playerExist != 0) {
        PlayerAutoTurn(player, redirects->enemyFirst);
      }
      int32_t ctrl = EvalExpressionI(player, &(scData->params[1]), 1);
      if (ctrl != -1) {
        player->ctrl = (ctrl != 0);
      }
      int32_t anim = EvalExpressionI(player, &(scData->params[2]), 1);
      if (anim != -1) {
        if (anim < 0) {
          anim = 0;
          Warn(player, "attemted change to negative anim");
        }
        PlayerSelfAnim(player, anim);
      }
    }
      return 1;

    case ID_SC_SelfState: {
      // $0 = stateNo
      // $1 = ctrl
      // $2 = anim
      player->enforcedStateList = NULL;
      player->enforcedStateListBy = 0xffffffff;
      player->command = &(player->info->command);
      player->enforcedAnim = NULL;
      player->anim->useAnimInfoEx = player->info->anim;
      if (player->anim->useAnimInfoEx->info->maxAnimIndex <= player->anim->currentAnimIndex) {
        PlayerSelfAnim(player, 0);
      }
      int32_t stateNo = EvalExpressionI(player, &(scData->params[0]), 1);
      if (stateNo < 0) {
        stateNo = 0;
        Warn(player, "attempted to selfstate to negative state");
      }
      PlayerChangeState(player, stateNo);
      int32_t ctrl = EvalExpressionI(player, &(scData->params[1]), 1);
      if (ctrl != -1) {
        player->ctrl = (ctrl != 0);
      }
      int32_t anim = EvalExpressionI(player, &(scData->params[2]), 1);
      if (anim != -1) {
        if (anim < 0) {
          anim = 0;
          Warn(player, "attemted change to negative anim");
        }
        PlayerSelfAnim(player, anim);
      }
    }
      return 1;

    case ID_SC_Turn:
      PlayerTurnAround(player);
      return 0;

    case ID_SC_VarSet: {
      // $0 = value
      // $1 = index
      // $2 = type (0: var, 1: fvar, 3: sysvar, 4: sysfvar)
      uint32_t type = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 0);
      int32_t index = EvalExpressionI(player, &(scData->params[1]), 1);
      if (type < 2) {
        if (index < 0 || index > (type == 0 ? 59 : 39)) {
          VWarn(player, "set var out of range (%i)", index);
          return 0;
        }
      } else if (index < 0 || index > 4) {
        VWarn(player, "set sysvar out of range (%i)", index);
        return 0;
      }
      switch (type) {
        case 0:  // Var
          (player->var)[index] = EvalExpressionI(player, &(scData->params[0]), 1);
          break;
        case 1:  // FVar
          (player->fvar)[index] = EvalExpressionF(player, &(scData->params[0]));
          break;
        case 3:  // SysVar
          (player->sysvar)[index] = EvalExpressionI(player, &(scData->params[0]), 1);
          break;
        default:  // SysFVar
          (player->sysfvar)[index] = EvalExpressionF(player, &(scData->params[0]));
          break;
      }
    }
      return 0;

    case ID_SC_VarAdd: {
      // $0 = value
      // $1 = index
      // $2 = type (0: var, 1: fvar, 3: sysvar, 4: sysfvar)
      uint32_t type = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 0);
      int32_t index = EvalExpressionI(player, &(scData->params[1]), 1);
      if (type < 2) {
        if (index < 0 || index > (type == 0 ? 59 : 39)) {
          VWarn(player, "set var out of range (%i)", index);
          return 0;
        }
      } else if (index < 0 || index > 4) {
        VWarn(player, "set sysvar out of range (%i)", index);
        return 0;
      }
      switch (type) {
        case 0:  // Var
          (player->var)[index] += EvalExpressionI(player, &(scData->params[0]), 1);
          break;
        case 1:  // FVar
          (player->fvar)[index] += EvalExpressionF(player, &(scData->params[0]));
          break;
        case 3:  // SysVar
          (player->sysvar)[index] += EvalExpressionI(player, &(scData->params[0]), 1);
          break;
        default:  // SysFVar
          (player->sysfvar)[index] += EvalExpressionF(player, &(scData->params[0]));
          break;
      }
    }
      return 0;

    case ID_SC_VarRandom: {
      // $0 = min
      // $1 = index
      // $2 = max
      int32_t index = EvalExpressionI(player, &(scData->params[1]), 1);
      if (index >= 0 && index <= 59) {
        int32_t min = EvalExpressionI(player, &(scData->params[0]), 1);
        int32_t max = EvalExpressionI(player, &(scData->params[2]), 1);
        if (max < min) {
          int32_t temp = min;
          min = max;
          max = temp;
        }
        (player->var)[index] = (uint32_t)(rand()) % ((max - min) + 1) + min;
      } else {
        VWarn(player, "randomized var out of range (%i)", index);
      }
    }
      return 0;

    case ID_SC_VarRangeSet: {
      // $0 = first
      // $1 = last
      // $2 = type (0: var, 1: fvar)
      uint32_t type = (uint32_t)EvalExpressionI(player, &(scData->params[3]), 0);
      int32_t first = EvalExpressionI(player, &(scData->params[0]), 1);
      int32_t last = EvalExpressionI(player, &(scData->params[1]), 1);
      if (first < 0 || last > (type == 0 ? 59 : 39) || last < first) {
        VWarn(player, "set illegal var range (%i-%i)", first, last);
      } else {
        if (type == 0) {
          int32_t value = EvalExpressionI(player, &(scData->params[2]), 1);
          if (value == 0) {
            SecureZeroMemory(&(player->var[first]), (last - first + 1) * 0x4);
          } else {
            for (int32_t i = first; i <= last; ++i) {
              (player->var)[i] = value;
            }
          }
        } else {
          float fvalue = EvalExpressionF(player, &(scData->params[2]));
          if (fvalue == 0.0F) {
            SecureZeroMemory(&(player->fvar[first]), (last - first + 1) * 0x4);
          } else {
            for (int32_t i = first; i <= last; ++i) {
              (player->fvar)[i] = fvalue;
            }
          }
        }
      }
    }
      return 0;

    case ID_SC_LifeSet: {
      // $0 = value
      int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
      PlayerLifeSet(player, value);
    }
      return 0;

    case ID_SC_LifeAdd: {
      // $0 = value
      // $1 = kill
      // $2 = absolute
      uint32_t absolute = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 0);
      uint32_t kill = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
      PlayerTakeDamage(player, -value, kill, absolute);
    }
      return 0;

    case ID_SC_PowerSet: {
      // $0 = value
      int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
      PlayerPowerSet(redirects->teamFirst, value);
    }
      return 0;

    case ID_SC_PowerAdd: {
      // $0 = value
      int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
      PlayerPowerSet(redirects->teamFirst, *((redirects->teamFirst)->pPower) + value);
    }
      return 0;

    case ID_SC_CtrlSet:
      // $0 = ctrl
      player->ctrl = (EvalExpressionI(player, &(scData->params[0]), 1) != 0);
      return 0;

    case ID_SC_PosSet: {
      // $0 = xがないなら0
      // $1 = yがないなら0
      // $3 = x
      // $4 = y
      uint32_t hasX = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (hasX != 0) {
        float posX = EvalExpressionF(player, &(scData->params[3]));
        player->screenPosX = (posX + g->fStageAxisX + 160.0F);
      }
      uint32_t hasY = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      if (hasY != 0) {
        float posY = EvalExpressionF(player, &(scData->params[4]));
        player->posY = posY;
      }
    }
      return 0;

    case ID_SC_PosAdd: {
      // $0 = xがないなら0
      // $1 = yがないなら0
      // $3 = x
      // $4 = y
      uint32_t hasX = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (hasX != 0) {
        float posX = EvalExpressionF(player, &(scData->params[3]));
        player->screenPosX += posX * player->facing;
      }
      uint32_t hasY = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      if (hasY != 0) {
        float posY = EvalExpressionF(player, &(scData->params[4]));
        player->posY += posY;
      }
    }
      return 0;

    case ID_SC_PosFreeze:
      // $0 = value
      player->posFreeze = (EvalExpressionI(player, &(scData->params[0]), 1) != 0);
      return 0;

    case ID_SC_VelSet: {
      // $0 = xがないなら0
      // $1 = yがないなら0
      // $3 = x
      // $4 = y
      uint32_t hasX = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (hasX != 0) {
        float velX = EvalExpressionF(player, &(scData->params[3]));
        player->velX = velX * player->facing;
      }
      uint32_t hasY = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      if (hasY != 0) {
        float velY = EvalExpressionF(player, &(scData->params[4]));
        player->velY = velY;
      }
    }
      return 0;

    case ID_SC_VelAdd: {
      // $0 = xがないなら0
      // $1 = yがないなら0
      // $3 = x
      // $4 = y
      uint32_t hasX = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (hasX != 0) {
        float velX = EvalExpressionF(player, &(scData->params[3]));
        player->velX += velX * player->facing;
      }
      uint32_t hasY = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      if (hasY != 0) {
        float velY = EvalExpressionF(player, &(scData->params[4]));
        player->velY += velY;
      }
    }
      return 0;

    case ID_SC_VelMul: {
      // $0 = xがないなら0
      // $1 = yがないなら0
      // $3 = x
      // $4 = y
      uint32_t hasX = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (hasX != 0) {
        float velX = EvalExpressionF(player, &(scData->params[3]));
        player->velX *= velX;
      }
      uint32_t hasY = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      if (hasY != 0) {
        float velY = EvalExpressionF(player, &(scData->params[4]));
        player->velY *= velY;
      }
    }
      return 0;

    case ID_SC_Gravity:
      player->velY += (player->consts).movement_YAccel;
      return 0;

    case ID_SC_Offset: {
      // $0 = xがないなら0
      // $1 = yがないなら0
      // $3 = x
      // $4 = y
      uint32_t hasX = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (hasX != 0) {
        float offsetX = EvalExpressionF(player, &(scData->params[3]));
        player->offsetX = (int32_t)offsetX;
      }
      uint32_t hasY = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
      if (hasY != 0) {
        float offsetY = EvalExpressionF(player, &(scData->params[4]));
        player->offsetY = (int32_t)offsetY;
      }
    }
      return 0;

    case ID_SC_HitBy: {
      // $0 = value
      // $1 = time
      // $2 = 0ならvalue, 0以外ならvalue2
      uint32_t no = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 0);
      if (no == 0) {
        player->hitBy = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
        player->hitByTime = EvalExpressionI(player, &(scData->params[1]), 0);
      } else {
        player->hitBy2 = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
        player->hitBy2Time = EvalExpressionI(player, &(scData->params[1]), 0);
      }
    }
      return 0;

    case ID_SC_ChangeAnim: {
      // $0 = value
      // $1 = elem
      int32_t animElemNo = EvalExpressionI(player, &(scData->params[1]), 1);
      int32_t animNo = EvalExpressionI(player, &(scData->params[0]), 1);
      if (animElemNo < 0) {
        animElemNo = 0;
        Warn(player, "changed to negative animelem");
      }
      if (animNo < 0) {
        animNo = 0;
        Warn(player, "attempted change to negative anim");
      }
      if (animElemNo != 0) {
        PlayerSelfAnimElem(player, animNo, animElemNo);
      } else {
        PlayerSelfAnim(player, animNo);
      }
    }
      return 0;

    case ID_SC_ChangeAnim2: {
      // $0 = value
      // $1 = elem
      if (player->enforcedAnim == NULL) {
        player->anim->useAnimInfoEx = player->info->anim;
      } else {
        player->anim->useAnimInfoEx = player->enforcedAnim;
      }
      int32_t animElemNo = EvalExpressionI(player, &(scData->params[1]), 1);
      int32_t animNo = EvalExpressionI(player, &(scData->params[0]), 1);
      if (animElemNo < 0) {
        animElemNo = 0;
        Warn(player, "changed to negative animelem");
      }
      if (animNo < 0) {
        animNo = 0;
        Warn(player, "attempted change to negative anim");
      }
      if (animElemNo != 0) {
        PlayerSelfAnimElem(player, animNo, animElemNo);
      } else {
        PlayerSelfAnim(player, animNo);
      }
    }
      return 0;

    case ID_SC_PlaySnd:
      SCtrlDoPlaySndInfo(player, (scData->paramsEx).playSnd);
      return 0;

    case ID_SC_StopSnd: {
      // $0 = channel
      int32_t channel = EvalExpressionI(player, &(scData->params[0]), 1);
      if (channel > -2 && channel < 32) {
        SndIDStop(PlayerSndID(player, channel));
      } else {
        VWarn(player, "stopped illegal channel number %i", channel);
      }
    }
      return 0;

    case ID_SC_SndPan: {
      // $0 = channel
      // $1 = pan
      // $2 = 0: pan, Otherwise: abspan
      int32_t channel = EvalExpressionI(player, &(scData->params[0]), 1);
      if (channel > -1 && channel < 32) {
        int32_t sndId = PlayerSndID(player, channel);
        if (sndId != -1) {
          int32_t index = SndIDGetChannel(sndId);
          if (index != -1) {
            int32_t pan = EvalExpressionI(player, &(scData->params[1]), 1) * player->facing;
            uint32_t panType = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
            if (panType == 0) {
              pan = (int32_t)(player->screenPosX - g->iStageAxisX + pan);
            } else {
              pan += 160;
            }
            SndVoiceSetPan(index, ScrPositionToPan(pan));
          }
        }
      } else {
        VWarn(player, "attempted to pan illegal channel number %i", channel);
      }
    }
      return 0;

    case ID_SC_HitDef:
    case ID_SC_ReversalDef: {
      int32_t guard_dist = (player->hitDefParams).guard_dist;
      int32_t juggle = (player->hitDefParams).jugglePoints;
      SCtrlInitHitDef((scData->paramsEx).hitDef, &(player->hitDefParams), player);
      (player->hitDefParams).isAttacking = 1;
      if (scData->scID == ID_SC_ReversalDef) {
        (player->hitDefParams).isHitDef = 0;
      }
      (player->hitDefParams).jugglePoints = juggle;
      int32_t damage = (int32_t)((float)((player->hitDefParams).damage) * (player->attackMul) + 0.5F);
      if ((player->hitDefParams).damage > 0 && damage == 0) {
        damage = 1;
      }
      (player->hitDefParams).damage = damage;
      if ((player->hitDefParams).guard_dist == -1) {
        (player->hitDefParams).guard_dist = guard_dist;
      }
      (player->hitDefParams).attackNo = (g->nextAttackNo)++;
    }
      return 0;

    case ID_SC_Projectile: {
      MUGEN_PLAYER* owner = player;
      // 一応ヘルパーもprojectilesは確保・初期化されているのだが,
      // この記述により意図的にroot以外のprojectile領域を
      // 使用しないようになっている => OTHK, 混線, etc はやはり仕様だった？
      if (redirects->root != NULL) {
        owner = redirects->root;
      }
      int32_t index = SCtrlInitProjectile((scData->paramsEx).projectile, player, redirects->enemyFirst, owner);
      if (index != -1) {
        if (index < 0 || index >= owner->projectiles->projInfo->maxProjCnt) {
          sprintf((char*)DebugTmpStr, "Assert failure in array.h line 110");
          DebugErrorExit((char*)DebugTmpStr);
        } else {
          MUGEN_PROJECTILE_DATA* data = &((owner->projectiles->projInfo)->data[index]);
          if (data != NULL) {
            PlayerAISctrlProjectile(player, data);
            int32_t damage = (int32_t)((float)((data->projHitDefParams).damage) * (player->attackMul) + 0.5F);
            if ((data->projHitDefParams).damage > 0 && damage == 0) {
              damage = 1;
            }
          }
        }
      }
    }
      return 0;

    case ID_SC_Width:
      // $0 = edgeFront
      // $1 = edgeBack
      // $2 = playerFront
      // $3 = playerBack
      player->widthEdgeFront = EvalExpressionI(player, &(scData->params[0]), 1);
      player->widthEdgeBack = EvalExpressionI(player, &(scData->params[1]), 1);
      player->widthPlayerFront = EvalExpressionI(player, &(scData->params[2]), 1);
      player->widthPlayerBack = EvalExpressionI(player, &(scData->params[3]), 1);
      return 0;

    case ID_SC_ScreenBound:
      // $0 = value
      // $1 = moveCamera X
      // $2 = moveCamera Y
      player->screenBound = (EvalExpressionI(player, &(scData->params[0]), 1) != 0);
      player->moveCameraX = (EvalExpressionI(player, &(scData->params[1]), 1) != 0);
      player->moveCameraY = (EvalExpressionI(player, &(scData->params[2]), 1) != 0);
      return 0;

    case ID_SC_SprPriority:
      // $0 = sprPriority
      player->sprPriority = EvalExpressionI(player, &(scData->params[0]), 1);
      return 0;

    case ID_SC_AngleDraw: {
      // $3 = value
      // $4 = scale X
      // $5 = scale Y
      player->angleDrawFlag = 1;
      float angle = EvalExpressionF(player, &(scData->params[3]));
      if (angle != -32760.0F) {
        player->angle = angle;
      }
      player->scaleX *= EvalExpressionF(player, &(scData->params[4]));
      player->scaleY *= EvalExpressionF(player, &(scData->params[5]));
    }
      return 0;

    case ID_SC_AngleSet:
      // $3 = value
      player->angle = EvalExpressionF(player, &(scData->params[3]));
      return 0;

    case ID_SC_AngleAdd:
      // $3 = value
      player->angle += EvalExpressionF(player, &(scData->params[3]));
      return 0;

    case ID_SC_AngleMul:
      // $3 = value
      player->angle *= EvalExpressionF(player, &(scData->params[3]));
      return 0;

    case ID_SC_Trans: {
      // $0 = trans
      // $1 = alphaAS
      // $2 = alphaD
      player->trans = (MUGEN_TRANS_TYPE)EvalExpressionI(player, &(scData->params[0]), 1);
      int32_t alphaAS = EvalExpressionI(player, &(scData->params[1]), 1);
      if (alphaAS < 0) {
        alphaAS = 0;
      } else if (alphaAS > 256) {
        alphaAS = 256;
      }
      player->alphaAS = alphaAS;
      int32_t alphaD = EvalExpressionI(player, &(scData->params[2]), 1);
      if (alphaD < 0) {
        alphaD = 0;
      } else if (alphaD > 256) {
        alphaD = 256;
      }
      player->alphaD = alphaD;
    }
      return 0;

    case ID_SC_TargetBind: {
      // $0 = time
      // $2 = id
      // $3 = pos X
      // $4 = pos Y
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        // targetの回数だけidが評価される
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          // time, pos X, pos Yは有効なtargetの数だけ評価される
          // (idを-1にしておけばidとtimeの評価回数の差からtarget中のガーステ固定キャラ数を把握可能
          //  = ステコンオバフロなしでキャラがガーステ効果中かを知ることが可能)
          int32_t time = EvalExpressionI(player, &(scData->params[0]), 1);
          if (time < -1) {
            Warn(player, "set illegal target bindtime");
            time = 1;
          }
          data->bindTime = time;
          float posX = EvalExpressionF(player, &(scData->params[3]));
          if (posX != -32760.0F) {
            data->bindPosX = (int32_t)posX;
          }
          float posY = EvalExpressionF(player, &(scData->params[4]));
          if (posY != -32760.0F) {
            data->bindPosY = (int32_t)posY;
          }
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_BindToTarget: {
      // $0 = time
      // $1 = 1: Mid, 2: Head, Otherwise: Foot
      // $2 = id
      // $3 = pos X
      // $4 = pos Y
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      // TargetBindと異なり, idが一度しか評価されない
      uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
      while (data != NULL) {
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          uint32_t baseType = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
          int32_t basePosX = 0;
          int32_t basePosY = 0;
          if (baseType == 1) {
            basePosX = (p->consts).size_mid_pos_x;
            basePosY = (p->consts).size_mid_pos_y;
          } else {
            // baseTypeはHead, Footでは2度評価される (が, Head/Mid/Footは必ず定数なので意味無し)
            baseType = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
            if (baseType == 2) {
              basePosX = (p->consts).size_head_pos_x;
              basePosY = (p->consts).size_head_pos_y;
            }
          }
          int32_t time = EvalExpressionI(player, &(scData->params[0]), 1);
          if (time < -1) {
            Warn(player, "set illegal target bindtime");
            time = 1;
          }
          data->bindTime = time;
          float posX = EvalExpressionF(player, &(scData->params[3]));
          if (posX != -32760.0F) {
            player->screenPosX = (p->facing * (posX + basePosX) + p->screenPosX);
            data->bindPosX = (int32_t)(player->facing * p->facing * (posX + basePosX));
          }
          float posY = EvalExpressionF(player, &(scData->params[4]));
          if (posY != -32760.0F) {
            player->posY = (posY + basePosY + p->posY);
            data->bindPosY = (int32_t)(posY + basePosY);
          }
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetState: {
      // $0 = stateNo
      // $2 = id
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          p->enforcedStateList = player->usingStateList;
          p->enforcedStateListBy = player->slotId;
          p->command = player->command;
          if (player->enforcedStateList != NULL) {
            p->enforcedStateList = player->enforcedStateList;
            p->enforcedStateListBy = player->enforcedStateListBy;
          }
          p->enforcedAnim = (player->enforcedAnim ? player->enforcedAnim : player->anim->useAnimInfoEx);
          int32_t stateNo = EvalExpressionI(player, &(scData->params[0]), 1);
          if (stateNo < 0) {
            stateNo = 0;
            Warn(player, "attempted targetstate to negative");
          }
          PlayerChangeState(p, stateNo);
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetPowerAdd: {
      // $0 = value
      // $2 = id
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          PlayerPowerSet(p, *(p->pPower) + value);
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetLifeAdd: {
      // $0 = value
      // $1 = kill
      // $2 = absolute
      // $5 = id
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[5]), 1);
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          uint32_t absolute = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 0);
          uint32_t kill = (uint32_t)EvalExpressionI(player, &(scData->params[1]), 0);
          int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
          PlayerTakeDamage(p, -value, kill, absolute);
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetVelSet: {
      // $2 = id
      // $3 = vel X
      // $4 = vel Y
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          float velX = EvalExpressionF(player, &(scData->params[3]));
          if (velX != -32760.0F) {
            p->velX = (p->facing * velX);
          }
          float velY = EvalExpressionF(player, &(scData->params[4]));
          if (velY != -32760.0F) {
            p->velY = velY;
          }
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetVelAdd: {
      // $2 = id
      // $3 = vel X
      // $4 = vel Y
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[2]), 1);
        MUGEN_PLAYER* p = data->player;
        if (p->isGuarding == 0 && (id == 0xffffffff || id == (p->getHitVars).chainId)) {
          float velX = EvalExpressionF(player, &(scData->params[3]));
          if (velX != -32760.0F) {
            p->velX += (p->facing * velX);
          }
          float velY = EvalExpressionF(player, &(scData->params[4]));
          if (velY != -32760.0F) {
            p->velY += velY;
          }
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetFacing: {
      // $0 = value
      // idが使用されていない？
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      while (data != NULL) {
        MUGEN_PLAYER* p = data->player;
        int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
        // 他のtarget系ステコンについているp->isGuardingの条件がない
        // = targetfacingのみガード効果を貫通する
        if ((value > 0 && player->facing != p->facing) || (value < 0 && (player->facing == p->facing))) {
          PlayerTurnAround(p);
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_TargetDrop: {
      // $0 = time
      // $1 = keepOne
      MUGEN_TARGET_DATA* data = (MUGEN_TARGET_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->targets));
      bool keeping = false;
      while (data != NULL) {
        MUGEN_PLAYER* p = data->player;
        uint32_t id = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 1);
        int32_t keepOne = EvalExpressionI(player, &(scData->params[1]), 1);
        if (id == (p->getHitVars).chainId && !keeping) {
          if (keepOne != 0) {
            keeping = true;
          }
        } else {
          ArrayDelete((ABSTRUCT_ARRAY_INFO*)(player->targets), (player->targets)->scaned_index);
        }
        data = (MUGEN_TARGET_DATA*)ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->targets));
      }
    }
      return 0;

    case ID_SC_AttackDist:
      // $0 = value
      (player->hitDefParams).guard_dist = EvalExpressionI(player, &(scData->params[0]), 1);
      return 0;

    case ID_SC_PlayerPush:
      // $0 = value
      player->playerPush = (EvalExpressionI(player, &(scData->params[0]), 1) != 0);
      return 0;

    case ID_SC_HitFallSet: {
      // $0 = value
      // $3 = XVel
      // $4 = YVel?
      uint32_t value = (uint32_t)EvalExpressionI(player, &(scData->params[0]), 0);
      if (value != 0xffffffff) {
        (player->getHitVars).fall = value;
      }
      float xVel = EvalExpressionF(player, &(scData->params[3]));
      if (xVel != -32760.0F) {
        (player->getHitVars).fall_XVel = xVel;
      }
      // XVel と YVel で同じ式を評価している
      // おそらくは $4 が正しい YVel の式のはず...
      float yVel = EvalExpressionF(player, &(scData->params[3]));
      if (yVel != -32760.0F) {
        (player->getHitVars).fall_YVel = yVel;
      }
    }
      return 0;

    case ID_SC_HitVelSet: {
      // $0 = x
      // $1 = y
      int32_t x = EvalExpressionI(player, &(scData->params[0]), 0);
      if (x != 0) {
        player->velX = (player->getHitVars).XVelAdd + (player->getHitVars).XVel;
      }
      int32_t y = EvalExpressionI(player, &(scData->params[1]), 0);
      if (y != 0) {
        player->velY = (player->getHitVars).YVelAdd + (player->getHitVars).YVel;
      }
    }
      return 0;

    case ID_SC_HitFallDamage:
      PlayerTakeDamage(player, (player->getHitVars).fall_damage, 1, 1);
      (player->getHitVars).fall_damage = 0;
      return 0;

    case ID_SC_HitFallVel:
      if ((player->getHitVars).fall_XVel != -32760.0F) {
        player->velX = (float)(player->facing * (player->getHitVars).fall_XVel);
      }
      if ((player->getHitVars).fall_YVel != -32760.0F) {
        player->velY = (player->getHitVars).fall_YVel;
      }
      return 0;

    case ID_SC_HitAdd: {
      // $0 = value
      int32_t value = EvalExpressionI(player, &(scData->params[0]), 1);
      player->internalHitCount += value;
      player->hitCount += value;
      player->uniqHitCount += value;
      ComboAdd(player->teamSide, value);
    }
      return 0;

    case ID_SC_AttackMulSet: {
      // $3 = value
      float value = EvalExpressionF(player, &(scData->params[3]));
      player->attackMul = (float)(value * (player->consts).data_attack * 0.009999999776482582F);
    }
      return 0;

    case ID_SC_DefenceMulSet: {
      // $3 = value
      float value = EvalExpressionF(player, &(scData->params[3]));
      player->defenceMul = (float)((value * 100.0F) / (player->consts).data_defence);
    }
      return 0;

    case ID_SC_StateTypeSet: {
      // $0 = stateType
      // $1 = moveType
      // $2 = physics
      MUGEN_PLAYER_STATETYPE stateType = (MUGEN_PLAYER_STATETYPE)EvalExpressionI(player, &(scData->params[0]), 0);
      if (stateType != STATETYPE_U) {
        player->stateType = stateType;
      }
      MUGEN_PLAYER_MOVETYPE moveType = (MUGEN_PLAYER_MOVETYPE)EvalExpressionI(player, &(scData->params[1]), 0);
      if (moveType != MOVETYPE_U) {
        player->moveType = moveType;
      }
      MUGEN_PLAYER_PHYSICS physics = (MUGEN_PLAYER_PHYSICS)EvalExpressionI(player, &(scData->params[2]), 0);
      if (physics != PHYSICS_U) {
        player->physics = physics;
      }
    }
      return 0;

    default:
      sprintf((char*)(g->errorMessage), "PlayerSCtrlApplyElem not recognised");
      ErrorExit();
      return 0;
  }
}

uint32_t R_SCtrlInitHitOverride(MUGEN_SC_PARAMS_EX_HITOVERRIDE* scParamsEx, MUGEN_HIT_OVERRIDE* hitOverride, MUGEN_PLAYER* player) {
  hitOverride->enable = 1;
  hitOverride->attr = scParamsEx->attr;
  int32_t stateNo = EvalExpressionI(player, &(scParamsEx->stateNo), 1);
  if (stateNo < -1) {
    stateNo = -1;
    Warn(player, "Illegal hitoverride stateno.");
  }
  hitOverride->state = stateNo;
  int32_t time = EvalExpressionI(player, &(scParamsEx->time), 1);
  if (time < -1) {
    time = 1;
    Warn(player, "Illegal hitoverride time.");
  }
  hitOverride->time = time;
  hitOverride->forceAir = EvalExpressionI(player, &(scParamsEx->forceAir), 1);
  return 1;
}

uint32_t R_SCtrlInitEnvShake(MUGEN_SC_PARAMS_EX_ENVSHAKE* paramsEx, MUGEN_ENV_SHAKE* envShake, MUGEN_PLAYER* player) {
  // dirパラメータは用意されてはいるものの評価されず
  SecureZeroMemory(envShake, 0x18);
  envShake->time = EvalExpressionI(player, &(paramsEx->time), 1);
  envShake->ampl = EvalExpressionF(player, &(paramsEx->ampl));
  envShake->phase = EvalExpressionF(player, &(paramsEx->phase));
  float freq = EvalExpressionF(player, &(paramsEx->freq));
  if (freq == 0.0F) {
    freq = 0.0010000000474974513F;
  } else if (freq < 0.0F) {
    Warn(player, "negative freq for EnvShake");
    freq = (freq * -1.0F);
  }
  envShake->freq = freq;
  if (paramsEx->_unknown != 0 && freq >= 90.0F) {
    envShake->phase = 90.0F;
  }
  envShake->elapsedTime = 0;
  return 1;
}

uint32_t R_SCtrlInitAftImgParams(MUGEN_SC_PARAMS_EX_AFTERIMAGE* paramsEx, MUGEN_AFTERIMAGE_DATA* afterImage, MUGEN_ANIM_USED* anim, MUGEN_PLAYER* player) {
  value32_t initData[24] = {0};
  initData[0].i = EvalExpressionI(player, &(paramsEx->time), 1);
  if (initData[0].i < -1) {
    initData[0].i = 1;
    Warn(player, "illegal time for afterimage");
  }
  int32_t length = EvalExpressionI(player, &(paramsEx->length), 1);
  if (length < 1) {
    initData[1].i = 20;
    Warn(player, "illegal length for afterimage");
  } else {
    if (length > afterImage->length_maximum) {
      initData[1].i = afterImage->length_maximum;
      VWarn(player, "length for afterimage exceeds maximum of %i", afterImage->length_maximum);
    } else {
      initData[1].i = length;
    }
  }
  initData[2].i = EvalExpressionI(player, &(paramsEx->timeGap), 1);
  if (initData[2].i < 1) {
    initData[2].i = 1;
    Warn(player, "illegal timegap for afterimage");
  }
  initData[3].i = EvalExpressionI(player, &(paramsEx->frameGap), 1);
  if (initData[3].i < 1) {
    initData[3].i = 4;
    Warn(player, "illegal framegap for afterimage");
  }
  initData[4].i = EvalExpressionI(player, &(paramsEx->palColor), 1);
  initData[5].i = EvalExpressionI(player, &(paramsEx->palInvertAll), 1);
  initData[6].i = EvalExpressionI(player, &(paramsEx->palBright_R), 1);
  initData[7].i = EvalExpressionI(player, &(paramsEx->palBright_G), 1);
  initData[8].i = EvalExpressionI(player, &(paramsEx->palBright_B), 1);
  initData[9].i = EvalExpressionI(player, &(paramsEx->palContrast_R), 1);
  initData[10].i = EvalExpressionI(player, &(paramsEx->palContrast_G), 1);
  initData[11].i = EvalExpressionI(player, &(paramsEx->palContrast_B), 1);
  initData[12].i = EvalExpressionI(player, &(paramsEx->palPostBright_R), 1);
  initData[13].i = EvalExpressionI(player, &(paramsEx->palPostBright_G), 1);
  initData[14].i = EvalExpressionI(player, &(paramsEx->palPostBright_B), 1);
  initData[15].i = EvalExpressionI(player, &(paramsEx->palAdd_R), 1);
  initData[16].i = EvalExpressionI(player, &(paramsEx->palAdd_G), 1);
  initData[17].i = EvalExpressionI(player, &(paramsEx->palAdd_B), 1);
  initData[18].f = EvalExpressionF(player, &(paramsEx->palMul_R));
  initData[19].f = EvalExpressionF(player, &(paramsEx->palMul_G));
  initData[20].f = EvalExpressionF(player, &(paramsEx->palMul_B));
  initData[21].u = (uint32_t)(paramsEx->trans);
  initData[22].i = EvalExpressionI(player, &(paramsEx->alphaAS), 1);
  initData[23].i = EvalExpressionI(player, &(paramsEx->alphaD), 1);
  AftImgSetup(afterImage, initData, anim);
  return 1;
}

void R_AftImgSetup(MUGEN_AFTERIMAGE_DATA* afterImage, int32_t data[static 24], MUGEN_ANIM_USED* anim) {
  afterImage->_unknown_2 = 0;
  afterImage->_unknown_3 = 0;
  afterImage->elapsedTime = 0;
  afterImage->anim = anim;
  afterImage->enable = 1;
  memcpy(&(afterImage->time), data, 24 * sizeof(int32_t));
}

uint32_t R_SCtrlInitPause(MUGEN_SC_PARAMS_EX_PAUSE* paramsEx, uint32_t pauseType, MUGEN_PLAYER* player) {
  int32_t time = EvalExpressionI(player, &(paramsEx->time), 1);
  if (time < 0) {
    Warn(player, "set illegal pausetime");
    time = 0;
  }
  int32_t moveTime = EvalExpressionI(player, &(paramsEx->moveTime), 1);
  if (moveTime < 0 || moveTime > time) {
    Warn(player, "set illegal pausemovetime");
    moveTime = 0;
  }
  uint32_t pauseBG = (uint32_t)EvalExpressionI(player, &(paramsEx->pauseBG), 1);
  int32_t endCmdBufTime = EvalExpressionI(player, &(paramsEx->endCmdBufTime), 1);
  if (endCmdBufTime < 0 || endCmdBufTime > time) {
    Warn(player, "set illegal endcmdbuftime");
    endCmdBufTime = 0;
  }

  // 実際はsc_pause, sc_superPauseを纏めて配列とし,
  // pauseTypeをインデックスとして分岐無しで代入している
  // -> pauseTypeが（隔離で）0, 1以外になるとメモリ破壊を起こす
  if (pauseType == 0) {
    // Pause
    (g->sc_pause).time = time;
    player->pauseMoveTime = moveTime;
    (g->sc_pause).pauseBG = pauseBG;
    (g->sc_pause).endCmdBufTime = endCmdBufTime;
  } else {
    // SuperPause
    (g->sc_superPause).time = time;
    player->superMoveTime = moveTime;
    (g->sc_superPause).pauseBG = pauseBG;
    (g->sc_superPause).endCmdBufTime = endCmdBufTime;
  }
  return 1;
}

uint32_t R_SCtrlInitPalFX(MUGEN_SC_PARAMS_EX_PALFX* paramsEx, MUGEN_PAL_FX* palFX, MUGEN_PLAYER* player) {
  MUGEN_PAL_FX* pal;
  if (paramsEx->flag == 1) {
    pal = (MUGEN_PAL_FX*)(&(g->bgPalFX));
  } else if (paramsEx->flag == 2) {
    pal = (MUGEN_PAL_FX*)(&(g->allPalFX));
  } else {
    pal = palFX;
  }
  SecureZeroMemory(pal, sizeof(MUGEN_PAL_FX));
  int32_t time = EvalExpressionI(player, &(paramsEx->time), 1);
  if (time < -1) {
    time = 0;
    Warn(player, "invalid PalFX time");
  }
  pal->time = time;
  int32_t color = EvalExpressionI(player, &(paramsEx->color), 1);
  pal->color = color;
  if (color < 0 || color > 256) {
    // 範囲外の場合の修正がされていない
    Warn(player, "invalid PalFX color value");
  }
  pal->addR = EvalExpressionI(player, &(paramsEx->add_R), 1);
  pal->addG = EvalExpressionI(player, &(paramsEx->add_G), 1);
  pal->addB = EvalExpressionI(player, &(paramsEx->add_B), 1);
  int32_t mulR = EvalExpressionI(player, &(paramsEx->mul_R), 1);
  pal->mulR = mulR;
  int32_t mulG = EvalExpressionI(player, &(paramsEx->mul_G), 1);
  pal->mulG = mulG;
  int32_t mulB = EvalExpressionI(player, &(paramsEx->mul_B), 1);
  pal->mulB = mulB;
  if (mulR < 0 || mulG < 0 || mulB < 0) {
    // 範囲外の場合の修正がされていない
    Warn(player, "invalid PalFX mul value");
  }
  pal->sinAddR = EvalExpressionI(player, &(paramsEx->sinAdd_R), 1);
  pal->sinAddG = EvalExpressionI(player, &(paramsEx->sinAdd_G), 1);
  pal->sinAddB = EvalExpressionI(player, &(paramsEx->sinAdd_B), 1);
  pal->sinAddF = EvalExpressionI(player, &(paramsEx->sinAdd_F), 1);
  pal->invertAll = (uint32_t)EvalExpressionI(player, &(paramsEx->invertAll), 1);
  return 1;
}

void R_ExplodMakeFromInfo(MUGEN_PLAYER* player, MUGEN_SC_PARAMS_EX_EXPLOD* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects) {
  int32_t animNo = EvalExpressionI(player, &(paramsEx->anim), 1);
  if (animNo < 0) {
    animNo = 0;
    Warn(player, "illegal explod animno");
  }
  MUGEN_SFF_INFO_EX* sff;
  MUGEN_ANIM_INFO_EX* anim;
  if (paramsEx->animFlag == 0) {
    if (EvalExpressionI(player, &(paramsEx->ownPal), 1) == 0) {
      sff = player->sff;
      anim = player->info->anim;
    } else {
      sff = player->commonPalleteSff;
      anim = player->info->anim;
    }
  } else {
    sff = g->fightfx_sff;
    anim = g->fightfx_anim;
  }
  if (ArrayFind((ABSTRUCT_ARRAY_INFO*)(anim->info), 0, animNo) == -1) {
    return;
  }
  int32_t max = g->explods->explodsMax;
  if (max < 1) {
    return;
  }
  int32_t cnt = 0;
  MUGEN_EXPLODS_DATA* data = (g->explods)->data;
  while (data->exists != 0) {
    ++cnt;
    ++data;
    if (max <= cnt) {
      return;
    }
  }
  ExplodDefault(data);
  data->visible = 1;
  data->anim->useAnimInfoEx = anim;
  data->anim->sff = sff;
  data->sff = sff;
  AnimChangeAction(data->anim, 0, animNo);
  AnimResetAction(data->anim);
  ++((g->explods)->explodsCnt);
  if (cnt != -1) {
    ExplodSetParams(player, data, paramsEx, redirects, 0);
  }
}

void R_ExplodDefault(MUGEN_EXPLODS_DATA* explod) {
  MUGEN_ANIM_USED* anim = explod->anim;
  SecureZeroMemory(explod, sizeof(MUGEN_EXPLODS_DATA));
  explod->anim = anim;
  explod->_unknown_3 = 0xffffffff;
  explod->exists = 1;
  explod->accelY = 0.0F;
  explod->accelX = 0.0F;
  explod->velY = 0.0F;
  explod->velX = 0.0F;
  explod->axisY = 0.0F;
  explod->axisX = 0.0F;
  explod->scaleY = 1.0F;
  explod->scaleX = 1.0F;
  explod->superMoveTime = 0;
  explod->pauseMoveTime = 0;
  explod->elapsedTime = 0;
  explod->removeTime = 0;
  explod->basePlayerRealPosY = NULL;
  explod->basePlayerRealPosX = NULL;
  explod->stagePosY = NULL;
  explod->stagePosX = NULL;
  explod->offsetY = 0;
  explod->offsetX = 0;
  explod->inverse = 0;
  explod->bindTime = 0;
  anim->spritesLayer = NULL;
  explod->anim->sprPriority = 0;
  explod->anim->useAnimInfoEx = NULL;
  explod->anim->sff = NULL;
  explod->trans = TRANS_DEFAULT;
  explod->transAlphaAS = 256;
  explod->transAlphaD = 0;
  explod->_unknown_8 = 0xffffffff;
  explod->_unknown_7 = 0xffffffff;
  explod->_unknown_6 = 0xffffffff;
  explod->_unknown_5 = 0xffffffff;
}

void R_ExplodSetParams(MUGEN_PLAYER* player, MUGEN_EXPLODS_DATA* explod, MUGEN_SC_PARAMS_EX_EXPLOD* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects, uint32_t isUpdate) {
  if (isUpdate == 0) {
    int32_t id = EvalExpressionI(player, &(paramsEx->id), 1);
    if (id < 0) {
      id = 0;
      Warn(player, "negative explod ID");
    }
    explod->id = id;
    explod->ownerPlayerId = player->playerId;
  }
  if (paramsEx->posType != 0xffff8011) {
    explod->_unknown_1 = 1;
    explod->_unknown_2 = 1;
    int32_t baseFacing;
    int32_t baseOffsetX;
    int32_t baseOffsetY;
    switch (paramsEx->posType) {
      case 0:  // P1
        baseFacing = player->facing;
        baseOffsetX = 0;
        baseOffsetY = 0;
        explod->basePlayerRealPosX = &(player->realScreenPosX);
        explod->basePlayerRealPosY = &(player->realScreenPosY);
        explod->_unknown_1 = 0;
        break;
      case 1:  // Front
        baseFacing = 1;
        baseOffsetX = ((player->facing == -1) ? 0 : 320);
        baseOffsetY = 0;
        explod->stagePosX = (int32_t*)(&(g->iStageAxisX));
        explod->stagePosY = (int32_t*)(&(g->iStageAxisY));
        break;
      case 2:  // Back
        baseFacing = player->facing;
        baseOffsetX = ((player->facing == 1) ? 0 : 320);
        baseOffsetY = 0;
        explod->stagePosX = (int32_t*)(&(g->iStageAxisX));
        explod->stagePosY = (int32_t*)(&(g->iStageAxisY));
        break;
      case 3:  // Left
        baseFacing = 1;
        baseOffsetX = 0;
        baseOffsetY = 0;
        explod->stagePosX = (int32_t*)(&(g->iStageAxisX));
        explod->stagePosY = (int32_t*)(&(g->iStageAxisY));
        break;
      case 4:  // Right
        baseFacing = 1;
        baseOffsetX = 320;
        baseOffsetY = 0;
        explod->stagePosX = (int32_t*)(&(g->iStageAxisX));
        explod->stagePosY = (int32_t*)(&(g->iStageAxisY));
        break;
      case 5:  // P2
        if (redirects->enemyFirst == NULL) {
          baseOffsetX = (int32_t)(player->realScreenPosX);
          baseOffsetY = (int32_t)(player->realScreenPosY);
          baseFacing = player->facing;
          explod->basePlayerRealPosX = &(player->realScreenPosX);
          explod->basePlayerRealPosY = &(player->realScreenPosY);
          explod->_unknown_1 = 0;
        } else {
          baseFacing = (redirects->enemyFirst)->facing;
          baseOffsetX = 0;
          baseOffsetY = 0;
          explod->basePlayerRealPosX = &((redirects->enemyFirst)->realScreenPosX);
          explod->basePlayerRealPosY = &((redirects->enemyFirst)->realScreenPosY);
          explod->_unknown_1 = 0;
        }
        break;
      default:
        // default時はbaseFacingが初期化されないが,
        // そもそも隔離抜きではdefaultを通り得ない
        baseOffsetX = 0;
        baseOffsetY = 0;
        break;
    }
    explod->posType = paramsEx->posType;
    int32_t facing = EvalExpressionI(player, &(paramsEx->facing), 1);
    if (facing != 1 && facing != -1) {
      Warn(player, "explod facing param must be 1 or -1");
      facing = 1;
    }
    if (facing * baseFacing == -1) {
      explod->inverse |= 0x1;
    }
    int32_t vFacing = EvalExpressionI(player, &(paramsEx->vFacing), 1);
    if (vFacing != 1 && vFacing == -1) {
      Warn(player, "explod vfacing param must be 1 or -1");
    }
    if (vFacing == -1) {
      explod->inverse |= 0x2;
    }
    explod->offsetX = baseOffsetX + EvalExpressionI(player, &(paramsEx->posX), 1) * baseFacing;
    explod->offsetY = baseOffsetY + EvalExpressionI(player, &(paramsEx->posY), 1);
    int32_t randomX = EvalExpressionI(player, &(paramsEx->randomX), 1);
    if (randomX < 0) {
      randomX = 0;
      Warn(player, "negative explod random param");
    }
    ++randomX;
    // random = 0, 0でもrand()は実行される
    explod->offsetX += ((uint32_t)(rand()) % randomX) - (randomX >> 1);
    int32_t randomY = EvalExpressionI(player, &(paramsEx->randomY), 1);
    if (randomY < 0) {
      randomY = 0;
      Warn(player, "negative explod random param");
    }
    ++randomY;
    explod->offsetY += ((uint32_t)(rand()) % randomY) - (randomY >> 1);
    explod->axisX = (float)(explod->offsetX);
    explod->axisY = (float)(explod->offsetY);
    explod->velX = EvalExpressionF(player, &(paramsEx->velX)) * baseFacing;
    explod->velY = EvalExpressionF(player, &(paramsEx->velY));
    explod->accelX = EvalExpressionF(player, &(paramsEx->accelX)) * baseFacing;
    explod->accelY = EvalExpressionF(player, &(paramsEx->accelY));
    if (isUpdate != 0 && explod->bindTime == 0) {
      explod->bindTime = 1;
    }
  }
  float scaleX = EvalExpressionF(player, &(paramsEx->scaleX));
  if (scaleX != -32760.0F) {
    explod->scaleX = scaleX;
  }
  float scaleY = EvalExpressionF(player, &(paramsEx->scaleY));
  if (scaleY != -32760.0F) {
    explod->scaleY = scaleY;
  }
  int32_t bindTime = EvalExpressionI(player, &(paramsEx->bindTime), 1);
  if (bindTime != -32751) {
    if (bindTime < -1) {
      bindTime = -1;
      Warn(player, "Illegal posbindtime for explod");
    }
    explod->bindTime = bindTime;
  }
  int32_t onTop = EvalExpressionI(player, &(paramsEx->onTop), 1);
  if (onTop != -32751) {
    explod->layerNo = ((onTop != 0) ? 2 : 0);
  }
  int32_t removeTime = EvalExpressionI(player, &(paramsEx->removeTime), 1);
  if (removeTime != -32751) {
    if (removeTime < -2) {
      Warn(player, "Illegal removetime for explod");
    }
    if (removeTime <= -2) {
      removeTime = AnimGetLooptime(explod->anim) - explod->elapsedTime;
    }
    explod->removeTime = removeTime;
  }
  int32_t superMoveTime = EvalExpressionI(player, &(paramsEx->superMoveTime), 1);
  if (superMoveTime == 0 || superMoveTime == -32751) {
    int32_t superMove = EvalExpressionI(player, &(paramsEx->superMove), 1);
    if (superMove != 0 && superMove != -32571) {
      superMoveTime = explod->removeTime;
      explod->superMoveTime = superMoveTime;
    } else if (superMove != -32571) {
      explod->superMoveTime = superMoveTime;
    } else {
      // (superMoveTime == 0 || superMoveTime == -32751) && superMove == -32751
      // skip updating the supermovetime
    }
  } else if (superMoveTime == -2) {
    superMoveTime = explod->removeTime;
    explod->superMoveTime = superMoveTime;
  } else {
    explod->superMoveTime = superMoveTime;
  }
  int32_t pauseMoveTime = EvalExpressionI(player, &(paramsEx->pauseMoveTime), 1);
  if (pauseMoveTime != -32571) {
    if (pauseMoveTime == -2) {
      pauseMoveTime = explod->removeTime;
    }
    explod->pauseMoveTime = pauseMoveTime;
  }
  if (explod->layerNo == 0) {
    // ontop != 0時はsprpriorityは評価されない？
    int32_t sprPriority = EvalExpressionI(player, &(paramsEx->sprPriority), 1);
    if (sprPriority != -32571) {
      explod->anim->sprPriority = sprPriority;
      explod->anim->spritesLayer = g->prioritySpritesLayer1;
    }
  } else {
    explod->anim->spritesLayer = NULL;
  }
  int32_t shadowR = EvalExpressionI(player, &(paramsEx->shadow_R), 1);
  if (shadowR != -32571) {
    int32_t shadowG = EvalExpressionI(player, &(paramsEx->shadow_G), 1);
    int32_t shadowB = EvalExpressionI(player, &(paramsEx->shadow_B), 1);
    if (shadowR < -1 || shadowG < -1 || shadowB < -1 || shadowR > 255 || shadowG > 255 || shadowB > 255) {
      Warn(player, "Illegal shadow color");
      explod->shadow = 0xffffffff;
    } else if (shadowR == -1) {
      // 何故かここだけ shadowG, shadowB の比較が抜けている
      explod->shadow = 0xffffffff;
    } else {
      explod->shadow = ((shadowB >> 3) << (_rgb_b_shift_16 & 0x1F)) | ((shadowG >> 2) << (_rgb_g_shift_16 & 0x1F)) | ((shadowR >> 3) << (_rgb_r_shift_16 & 0x1F));
    }
  }
  uint32_t removeOnGetHit = (uint32_t)EvalExpressionI(player, &(paramsEx->removeOnGetHit), 1);
  if (removeOnGetHit != 0xffff8011) {
    explod->removeOnGetHit = removeOnGetHit;
  }
  uint32_t ignoreHitPause = (uint32_t)EvalExpressionI(player, &(paramsEx->ignoreHitPause), 1);
  if (ignoreHitPause != 0xffff8011) {
    explod->ignoreHitPause = ignoreHitPause;
  }
  if ((uint32_t)(paramsEx->trans) != 0xffff8011) {
    explod->trans = paramsEx->trans;
  }
  int32_t alphaAS = EvalExpressionI(player, &(paramsEx->alphaAS), 1);
  if (alphaAS != -32571) {
    if (alphaAS < 0) {
      alphaAS = 0;
    } else if (alphaAS > 256) {
      alphaAS = 256;
    }
    explod->transAlphaAS = alphaAS;
  }
  int32_t alphaD = EvalExpressionI(player, &(paramsEx->alphaD), 1);
  if (alphaD != -32571) {
    if (alphaD < 0) {
      alphaD = 0;
    } else if (alphaD > 256) {
      alphaD = 256;
    }
    explod->transAlphaAS = alphaD;
  }
  return;
}

void R_ExplodBindChange(MUGEN_EXPLODS_INFO* explods, int32_t playerId, int32_t explodId, int32_t bindTime) {
  for (int32_t i = 0; i < explods->explodsMax; ++i) {
    MUGEN_EXPLODS_DATA* data = &(explods->data[i]);
    if (data->exists != 0 && (data->id == explodId || explodId == -1) && data->ownerPlayerId == playerId) {
      data->bindTime = bindTime;
    }
  }
}

void R_ExplodModifyFromInfo(MUGEN_PLAYER* player, MUGEN_SC_PARAMS_EX_EXPLOD* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects) {
  int32_t id = EvalExpressionI(player, &(paramsEx->id), 1);
  for (int32_t i = 0; i < (g->explods)->explodsMax; ++i) {
    MUGEN_EXPLODS_DATA* data = &((g->explods)->data[i]);
    if (data->exists != 0 && (data->id == id || id == -1) && data->ownerPlayerId == player->playerId) {
      ExplodSetParams(player, data, paramsEx, redirects, 1);
    }
  }
}

MUGEN_PLAYER* R_HelperMake(MUGEN_PLAYER* parent, MUGEN_SC_PARAMS_EX_HELPER* paramsEx, MUGEN_PLAYER_REDIRECTS* redirects) {
  if (g->totalSlots <= 4) {
    return NULL;
  }
  int32_t i = 5;
  MUGEN_PLAYER** pp = (MUGEN_PLAYER**)(&(g->playerList[5]));
  while ((*pp)->playerExist != 0) {
    ++i;
    ++pp;
    if (g->totalSlots < i) {
      return NULL;
    }
  }
  MUGEN_PLAYER* p = g->playerList[i];
  if (p == NULL) {
    return NULL;
  }
  p->info = parent->info;
  p->usingStateList = parent->usingStateList;
  p->command = &(parent->info->command);
  PlayerEnableSet(p->slotId, 1);
  int32_t ownPal = EvalExpressionI(parent, &(paramsEx->ownPal), 0);
  if (ownPal == 0) {
    p->sff = parent->sff;
  } else {
    if (SpriteArrayNewInstanceOnto(p->personalPalleteSff, parent->commonPalleteSff) == NULL) {
      return NULL;
    }
    p->sff = p->personalPalleteSff;
  }
  p->commonPalleteSff = parent->commonPalleteSff;
  p->anim->useAnimInfoEx = p->info->anim;
  p->anim->sff = p->sff;
  p->projectiles->animInfo = p->info->anim;
  p->projectiles->sffInfo = p->commonPalleteSff;
  if (ownPal != 0) {
    if (PalGrpFlagIsNonZero(p->palGroup) != 0) {
      PalGrpRemove(p->palGroup);
      PalGrpAddSpr(p->sff, p->palGroup);
    }
  }
  if ((paramsEx->name)[0] == '\0') {
    sprintf(p->debugName, "%s's helper", parent->debugName);
  } else {
    strncpy(p->debugName, paramsEx->name, sizeof(p->debugName));
  }
  p->playerId = (g->nextPlayerId)++;
  memcpy(&(p->consts), &(parent->consts), sizeof(MUGEN_PLAYER_CONSTRAINTS));
  float size_XScale = EvalExpressionF(parent, &(paramsEx->size_XScale));
  if (size_XScale != -32760.0F) {
    (p->consts).size_XScale = size_XScale;
  }
  float size_YScale = EvalExpressionF(parent, &(paramsEx->size_YScale));
  if (size_YScale != -32760.0F) {
    (p->consts).size_YScale = size_YScale;
  }
  int32_t size_ground_back = EvalExpressionI(parent, &(paramsEx->size_ground_back), 1);
  if (size_ground_back != -32751) {
    (p->consts).size_ground_back = size_ground_back;
  }
  int32_t size_ground_front = EvalExpressionI(parent, &(paramsEx->size_ground_front), 1);
  if (size_ground_front != -32751) {
    (p->consts).size_ground_front = size_ground_front;
  }
  int32_t size_air_back = EvalExpressionI(parent, &(paramsEx->size_air_back), 1);
  if (size_air_back != -32751) {
    (p->consts).size_air_back = size_air_back;
  }
  int32_t size_air_front = EvalExpressionI(parent, &(paramsEx->size_air_front), 1);
  if (size_air_front != -32751) {
    (p->consts).size_air_front = size_air_front;
  }
  int32_t size_height = EvalExpressionI(parent, &(paramsEx->size_height), 1);
  if (size_height != -32751) {
    (p->consts).size_height = size_height;
  }
  uint32_t size_proj_doScale = (uint32_t)EvalExpressionI(parent, &(paramsEx->size_proj_doScale), 1);
  if (size_proj_doScale != 0xffff8011) {
    (p->consts).size_proj_doScale = size_proj_doScale;
  }
  int32_t size_head_pos_x = EvalExpressionI(parent, &(paramsEx->size_head_pos_x), 1);
  if (size_head_pos_x != -32751) {
    (p->consts).size_head_pos_x = size_head_pos_x;
  }
  int32_t size_head_pos_y = EvalExpressionI(parent, &(paramsEx->size_head_pos_y), 1);
  if (size_head_pos_y != -32751) {
    (p->consts).size_head_pos_y = size_head_pos_y;
  }
  int32_t size_mid_pos_x = EvalExpressionI(parent, &(paramsEx->size_mid_pos_x), 1);
  if (size_mid_pos_x != -32751) {
    (p->consts).size_mid_pos_x = size_mid_pos_x;
  }
  int32_t size_mid_pos_y = EvalExpressionI(parent, &(paramsEx->size_mid_pos_y), 1);
  if (size_mid_pos_y != -32751) {
    (p->consts).size_mid_pos_y = size_mid_pos_y;
  }
  int32_t size_shadowOffset = EvalExpressionI(parent, &(paramsEx->size_shadowOffset), 1);
  if (size_shadowOffset != -32751) {
    (p->consts).size_shadowOffset = size_shadowOffset;
  }
  PlayerReset(p);
  SecureZeroMemory(p->var, sizeof(p->var));
  SecureZeroMemory(p->fvar, sizeof(p->fvar));
  int32_t stateNo = EvalExpressionI(parent, &(paramsEx->stateNo), 1);
  if (stateNo < 0) {
    stateNo = 0;
    Warn(parent, "illegal helper stateno");
  }
  PlayerChangeState(p, stateNo);
  p->screenPosX = parent->screenPosX;
  p->posY = parent->posY;
  p->teamSide = parent->teamSide;
  p->configKeys = parent->configKeys;
  int32_t keyCtrl = EvalExpressionI(parent, &(paramsEx->keyCtrl), 0);
  p->keyCtrl = (keyCtrl == 0);
  keyCtrl = EvalExpressionI(parent, &(paramsEx->keyCtrl), 0);
  if (keyCtrl != 0) {
    memcpy(&(p->keyInputs), &(parent->keyInputs), sizeof(MUGEN_KEY_INPUTS));
    memcpy(&(p->_unknown_9), &(parent->_unknown_9), sizeof(p->_unknown_9));
  }
  int32_t id = EvalExpressionI(parent, &(paramsEx->id), 1);
  if (id < 0) {
    id = 0;
    Warn(parent, "negative helper ID");
  }
  p->helperId = id;
  p->parentId = parent->playerId;
  p->parent = parent;
  if (parent->root == NULL) {
    p->root = parent;
  } else {
    p->root = parent->root;
  }
  p->playerType = paramsEx->playerType;
  p->facing = 1;
  int32_t basePosX;
  int32_t basePosY;
  switch (paramsEx->posType) {
    case 0:
      basePosX = (int32_t)(parent->screenPosX);
      basePosY = (int32_t)(parent->posY);
      p->facing = parent->facing;
      break;
    case 1:
      basePosX = g->iStageAxisX + ((parent->facing == -1) ? 0 : 320);
      basePosY = 0;
      p->facing = parent->facing;
      break;
    case 2:
      basePosX = g->iStageAxisX + ((parent->facing == 1) ? 0 : 320);
      basePosY = 0;
      p->facing = parent->facing;
      break;
    case 3:
      basePosX = g->iStageAxisX;
      basePosY = 0;
      break;
    case 4:
      basePosX = g->iStageAxisX + 320;
      basePosY = 0;
      break;
    case 5:
      if (redirects->enemyFirst == NULL) {
        basePosX = (int32_t)(parent->screenPosX);
        basePosY = (int32_t)(parent->posY);
        p->facing = parent->facing;
      } else {
        basePosX = (int32_t)((redirects->teamFirst)->screenPosX);
        basePosY = (int32_t)((redirects->teamFirst)->posY);
        p->facing = (redirects->teamFirst)->facing;
      }
      break;
  }
  p->screenPosX = (float)(EvalExpressionI(parent, &(paramsEx->posX), 1) * p->facing + basePosX);
  p->posY = (float)(basePosY + EvalExpressionI(parent, &(paramsEx->posY), 1));
  p->posZ = parent->posZ;
  if (EvalExpressionI(parent, &(paramsEx->facing), 1) == -1) {
    p->facing *= -1;
  }
  KBISetFlip(&(p->keyInputs), (p->facing == -1));
  if (p->playerType != 1) {
    p->screenBound = 0;
    p->moveCameraY = 0;
    p->moveCameraX = 0;
    p->playerPush = 0;
  }
  int32_t pauseMoveTime = EvalExpressionI(parent, &(paramsEx->pauseMoveTime), 1);
  if (pauseMoveTime < 0) {
    pauseMoveTime = 0;
    Warn(parent, "illegal helper pausemovetime");
  }
  p->pauseMoveTime = pauseMoveTime;
  int32_t superMoveTime = EvalExpressionI(parent, &(paramsEx->superMoveTime), 1);
  if (superMoveTime < 0) {
    superMoveTime = 0;
    Warn(parent, "illegal helper supermovetime");
  }
  p->superMoveTime = superMoveTime;
  p->hitBy = 0;
  p->hitByTime = 1;
  p->p2Enemy = PlayerNearestEnemy(p);
  SecureZeroMemory(&(p->assertSpecialFlags), 9);
  g->isRunnableSlot[p->slotId] = 1;
  g->isHelperSummoned = 1;
  return p;
}

void R_PlayerReset(MUGEN_PLAYER* player) {
  if (player->info == NULL) {
    return;
  }
  MUGEN_PLAYER* teamFirst = player;
  if (player->teamSide == 1) {
    teamFirst = g->playerList[1];
  } else if (player->teamSide == 2) {
    teamFirst = g->playerList[2];
  }
  player->isFrozen = 0;
  player->lifeMax = ((g->options).life * (player->consts).data_life) / 100;
  player->lifebarBack = ((g->options).life * (player->consts).data_life) / 100;
  player->lifebarFront = ((g->options).life * (player->consts).data_life) / 100;
  player->life = ((g->options).life * (player->consts).data_life) / 100;
  player->powerMax = (player->consts).data_power;
  player->pPowerMax = &(teamFirst->powerMax);
  player->pPower = &(teamFirst->power);
  player->attackMul = ((player->consts).data_attack * 0.01F);
  player->defenceMul = (100.0F / (player->consts).data_defence);
  player->powerbar = 0;
  player->power = 0;
  player->facing = 1;
  player->turnFlag = 0;
  player->isVisible = 1;
  player->velZ = 0.0F;
  player->velY = 0.0F;
  player->velX = 0.0F;
  player->offsetY = 0;
  player->offsetX = 0;
  player->offsetZ = 0;
  player->sprPriority = 0;
  player->leftAirJumpNum = 0;
  player->pauseMoveTime = 0;
  player->superMoveTime = 0;
  player->screenBound = 1;
  player->widthPlayerFront = 0;
  player->widthPlayerBack = 0;
  player->widthEdgeFront = 0;
  player->widthEdgeBack = 0;
  SecureZeroMemory(&(player->assertSpecialFlags), 9);
  player->projContactType = PROJCONTACT_NONE;
  player->projContactTime = 0;
  player->projContactId = 0;
  {
    MUGEN_PROJECTILE_DATA* data = (MUGEN_PROJECTILE_DATA*)ArrayScanFirst((ABSTRUCT_ARRAY_INFO*)(player->projectiles->projInfo));
    while (data != NULL) {
      ProjArrayDelete(player->projectiles, player->projectiles->projInfo->scaned_index);
      data = ArrayScanNext((ABSTRUCT_ARRAY_INFO*)(player->projectiles->projInfo));
    }
  }
  ArrayClear((ABSTRUCT_ARRAY_INFO*)(player->targets));
  SecureZeroMemory(&(player->_unknown_1), 0x18);
  player->_unknown_3 = 1.0F;
  player->_unknown_4 = 1.0F;
  SecureZeroMemory(&(player->keyInputs), sizeof(MUGEN_KEY_INPUTS));
  SecureZeroMemory(&(player->_unknown_9), sizeof(player->_unknown_9));
  player->_unknown_7 = 0;
  player->keyCtrl = 0;
  player->_unknown_8 = 0;
  player->command = &(player->info->command);
  KBClearCommandValidTimes(&(player->commandTimes));
  KBClearCommandLastTimes(&(player->commandTimes));
  KBISetFlip(&(player->keyInputs), 0);
  player->enforcedStateList = NULL;
  player->enforcedStateListBy = 0xffffffff;
  player->stateNo = 0;
  player->prevStateNo = 0;
  SecureZeroMemory(&(player->persistents), sizeof(player->persistents));
  player->time = 0;
  player->stateType = STATETYPE_S;
  player->moveType = MOVETYPE_I;
  player->physics = PHYSICS_S;
  player->ctrl = 1;
  player->isGuarding = 0;
  player->idleTime = 0;
  player->nextHitPauseTime = 0;
  player->hitPauseTime = 0;
  player->posFreeze = 0;
  player->alive = 1;
  player->timeSinceDeath = 0;
  player->gameTime = 0;
  player->moveContactType = MOVECONTACT_NONE;
  player->moveContactTime = 0;
  player->hasValidStateNo = 1;
  SecureZeroMemory(&(player->var), (player->consts).data_intPersistIndex * 4);
  SecureZeroMemory(&(player->fvar), (player->consts).data_floatPersistIndex * 4);
  SecureZeroMemory(&(player->sysvar), 5 * 4);
  SecureZeroMemory(&(player->sysfvar), 5 * 4);
  {
    int32_t* temp = player->_unknown_15;
    SecureZeroMemory(&(player->unhittable), 0x15c);
    player->_unknown_15 = temp;
  }
  player->hitBy2 = 0xfff;
  player->hitBy = 0xfff;
  (player->getHitVars).YAccel = 0.3499999940395355F;
  (player->getHitVars).recoverTime = (player->consts).data_lieDown_time;
  (player->getHitVars).fall_XVel = -32760.0F;
  (player->getHitVars).fall_YVel = -4.5F;
  for (int32_t i = 0; i < g->totalSlots; ++i) {
    (player->_unknown_15)[i] = -1;
  }
  for (int32_t i = g->totalSlots + 1; i != 0; --i) {
    (player->_unknown_18)[i] = 0;
  }
  SecureZeroMemory(&(player->hitDefParams), sizeof(MUGEN_HIT_DEF_PARAMS));
  SecureZeroMemory(&(player->scale), 19 * 4);
  player->scaleY = 1.0F;
  player->scaleX = 1.0F;
  player->_unknown_19 = -32000;
  player->trans = TRANS_DEFAULT;
  player->alphaAS = 256;
  player->alphaD = 0;
  if (player->afterImage != NULL) {
    player->afterImage->enable = 0;
    player->afterImage->anim = NULL;
    player->afterImage = NULL;
  }
  player->internalHitCount = 0;
  player->hitCount = 0;
  player->uniqHitCount = 0;
  player->_unknown_26 = 0;
  player->_unknown_27 = 0;
  player->enforcedAnim = NULL;
  if (player == NULL) {
    // 余りにも遅すぎるassert...
    sprintf((char*)DebugTmpStr, "Assert failure in p-update.c line 418");
    DebugErrorExit((char*)DebugTmpStr);
  }
  if (player->info->anim == NULL) {
    sprintf((char*)DebugTmpStr, "Assert failure in p-update.c line 418");
    DebugErrorExit((char*)DebugTmpStr);
  }
  player->anim->useAnimInfoEx = player->info->anim;
  PlayerChangeAnim(player, 0);
  AnimResetAction(player->anim);
  PalFXReset(&(player->palFX));
  PlayerResetAI(player);
  SecureZeroMemory(&(player->bindTo), sizeof(MUGEN_PLAYER_BIND));
  player->enemynear = NULL;
  player->p2Enemy = NULL;
  player->animInvalidFlag = 0;
  {
    MUGEN_CLIPBOARD* clipboard = player->clipboard;
    // ミス？
    player->clipboard = NULL;
    player->clipboard = clipboard;
    ClipboardClear(clipboard);
  }
}

void R_HelperDestroy(MUGEN_PLAYER* player) {
  if (player->isHelper != 1) {
    return;
  }
  PlayerDropBoundTargets(player, NULL);
  for (int32_t i = g->firstPlayerIndex; i <= g->lastPlayerIndex; ++i) {
    MUGEN_PLAYER* p = g->playerList[i];
    if (p != NULL && p->playerExist != 0 && (p->bindTo).player == player && (p->bindTo).time != 0) {
      (p->bindTo).time = 0;
      (p->bindTo).player = NULL;
    }
  }
  if (player->afterImage != NULL) {
    player->afterImage->enable = 0;
    player->afterImage->anim = NULL;
    player->afterImage = NULL;
  }
  if (PalGrpFlagIsNonZero(player->palGroup) != 0) {
    // 何故3回呼んでいるかは不明
    PalGrpRemove(player->palGroup);
    PalGrpRemove(player->palGroup);
    PalGrpRemove(player->palGroup);
  }
  player->commonPalleteSff = NULL;
  player->sff = NULL;
  PlayerEnableSet(player->slotId, 0);
}

void R_SCtrlDoPlaySndInfo(MUGEN_PLAYER* player, MUGEN_SC_PARAMS_EX_PLAYSND* paramsEx) {
  if ((g->soundWin2).sound == 0 || paramsEx == NULL || g->_unknown_91 != 0) {
    return;
  }
  MUGEN_SND_INFO** snd;
  if (paramsEx->sndFlag == 0) {
    snd = player->info->snd;
  } else {
    snd = g->common_snd;
  }
  if (snd == NULL) {
    return;
  }
  int32_t channel = EvalExpressionI(player, &(paramsEx->channel), 1);
  if (channel < -1 || channel > 31) {
    channel = -1;
    Warn(player, "Illegal playsnd channel.");
  }
  int32_t sndId = PlayerSndID(player, channel);
  if (EvalExpressionI(player, &(paramsEx->lowPriority), 1) != 0 && sndId != -1 && SndIDIsPlaying(sndId) != 0) {
    return;
  }
  int32_t groupNo = EvalExpressionI(player, &(paramsEx->sndGroupNo), 1);
  if (groupNo < -1) {
    groupNo = -1;
    Warn(player, "Illegal playsnd groupno.");
  }
  int32_t itemNo = EvalExpressionI(player, &(paramsEx->sndItemNo), 1);
  if (itemNo < -1) {
    itemNo = -1;
    Warn(player, "Illegal playsnd sndno.");
  }
  int32_t index = SndVoiceCleanup(sndId, snd, groupNo, itemNo);
  if (index == -1 || groupNo == -1) {
    return;
  }
  int32_t pan = EvalExpressionI(player, &(paramsEx->pan), 1) * player->facing;
  if (paramsEx->panType == 0) {
    pan = (uint32_t)(player->screenPosX - g->iStageAxisX + pan);
  } else {
    pan += 160;
  }
  if ((g->soundWin).stereoEffects == 0) {
    pan = 0;
  } else {
    pan = ScrPositionToPan(pan);
  }
  int32_t volume = EvalExpressionI(player, &(paramsEx->volume), 1);
  volume = SndCorrectVolOff(volume) + (g->soundWin2).wavVolume;
  if (paramsEx->sndFlag == 0) {
    volume += SndCorrectVolOff(player->info->volume);
  }
  volume = PanVolumeAdjust(volume);
  int32_t frequency = SndVoiceGetFreq(index);
  float freqMul = EvalExpressionF(player, &(paramsEx->freqMul));
  if (freqMul > 0.0F) {
    if (freqMul != 1.0F) {
      frequency = (int32_t)((float)frequency * freqMul);
      if (frequency <= 100) {
        frequency = 100;
      }
    }
  } else {
    Warn(player, "Non-positive freqmul.");
  }
  SndVoiceSetVol(index, volume);
  SndVoiceSetFreq(index, frequency);
  SndVoiceSetPan(index, pan);
  SndVoiceSetLoop(index, (EvalExpressionI(player, &(paramsEx->loop), 1) != 0));
  SndVoiceStart(index);
}

uint32_t R_SCtrlInitHitDef(MUGEN_SC_PARAMS_EX_HITDEF* paramsEx, MUGEN_HIT_DEF_PARAMS* hitDefParams, MUGEN_PLAYER* player) {
  hitDefParams->affectTeam = paramsEx->affectTeam;
  hitDefParams->attr = paramsEx->attr;

  {
    int32_t id = EvalExpressionI(player, &(paramsEx->id), 1);
    if (id < 0) {
      id = 0;
      Warn(player, "assigned negative hitID");
    }
    hitDefParams->hitId = id;
  }

  {
    int32_t chainId = EvalExpressionI(player, &(paramsEx->chainId), 1);
    if (chainId < -1) {
      chainId = -1;
      Warn(player, "specified negative chainID");
    }
    int32_t noChainId = EvalExpressionI(player, &(paramsEx->noChainId), 1);
    if (noChainId < -1) {
      noChainId = -1;
      Warn(player, "specified negative nochainID");
    }
    int32_t noChainId2 = EvalExpressionI(player, &(paramsEx->noChainId2), 1);
    if (noChainId2 < -1) {
      noChainId2 = -1;
      Warn(player, "specified negative nochainID2");
    }
    if (chainId != -1 && (chainId == noChainId || chainId == noChainId2)) {
      chainId = -1;
      noChainId = -1;
      noChainId2 = -1;
    }
    hitDefParams->chainId = chainId;
    hitDefParams->noChainId = noChainId;
    hitDefParams->noChainId2 = noChainId2;
  }

  hitDefParams->kill = (EvalExpressionI(player, &(paramsEx->kill), 1) != 0);
  hitDefParams->guard_kill = (EvalExpressionI(player, &(paramsEx->guard_kill), 1) != 0);
  hitDefParams->fall_kill = (EvalExpressionI(player, &(paramsEx->fall_kill), 1) != 0);
  hitDefParams->hitOnce = (EvalExpressionI(player, &(paramsEx->hitOnce), 1) != 0);

  {
    int32_t juggle = EvalExpressionI(player, &(paramsEx->juggle), 1);
    if (juggle < -1) {
      juggle = -1;
      Warn(player, "illegal juggle points");
    }
    hitDefParams->jugglePoints = juggle;
  }

  hitDefParams->damage = EvalExpressionI(player, &(paramsEx->damage), 1);
  hitDefParams->guard_damage = EvalExpressionI(player, &(paramsEx->guard_damage), 1);
  hitDefParams->getPower = EvalExpressionI(player, &(paramsEx->getPower), 0);
  hitDefParams->guard_getPower = EvalExpressionI(player, &(paramsEx->guard_getPower), 0);
  hitDefParams->givePower = EvalExpressionI(player, &(paramsEx->givePower), 0);
  hitDefParams->guard_givePower = EvalExpressionI(player, &(paramsEx->guard_givePower), 0);

  {
    int32_t p2HitPauseTime = EvalExpressionI(player, &(paramsEx->p2HitPauseTime), 1);
    if (p2HitPauseTime < 0) {
      p2HitPauseTime = 0;
      Warn(player, "specified negative p2shaketime");
    }
    hitDefParams->p2HitPauseTime = p2HitPauseTime;
  }
  {
    int32_t p1HitPauseTime = EvalExpressionI(player, &(paramsEx->p1HitPauseTime), 1);
    if (p1HitPauseTime < 0) {
      p1HitPauseTime = 0;
      Warn(player, "specified negative pausetime");
    }
    hitDefParams->p1HitPauseTime = p1HitPauseTime;
  }

  hitDefParams->numHits = EvalExpressionI(player, &(paramsEx->numHits), 1);

  {
    int32_t hitSoundGroupNo = EvalExpressionI(player, &(paramsEx->hitSound_groupNo), 1);
    if (hitSoundGroupNo < -1) {
      hitSoundGroupNo = -1;
      Warn(player, "specified illegal sound group");
    }
    (hitDefParams->hitSound).groupNo = hitSoundGroupNo;
  }
  {
    int32_t hitSoundItemNo = EvalExpressionI(player, &(paramsEx->hitSound_itemNo), 1);
    if (hitSoundItemNo < -1) {
      hitSoundItemNo = -1;
      Warn(player, "specified illegal sound no");
    }
    (hitDefParams->hitSound).itemNo = hitSoundItemNo;
  }
  hitDefParams->hitSoundFlag = paramsEx->hitSoundFlag;

  {
    int32_t guardSoundGroupNo = EvalExpressionI(player, &(paramsEx->guardSound_groupNo), 1);
    if (guardSoundGroupNo < -1) {
      guardSoundGroupNo = -1;
      Warn(player, "specified illegal gsound group");
    }
    (hitDefParams->guardSound).groupNo = guardSoundGroupNo;
  }
  {
    int32_t guardSoundItemNo = EvalExpressionI(player, &(paramsEx->guardSound_itemNo), 1);
    if (guardSoundItemNo < -1) {
      guardSoundItemNo = -1;
      Warn(player, "specified illegal gsound no");
    }
    (hitDefParams->guardSound).itemNo = guardSoundItemNo;
  }
  hitDefParams->guardSoundFlag = paramsEx->guardSoundFlag;

  hitDefParams->guardFlag = paramsEx->guardFlag;
  hitDefParams->hitFlag = paramsEx->hitFlag;

  {
    int32_t hitPriority = EvalExpressionI(player, &(paramsEx->hitPriority), 1);
    if (hitPriority < 0) {
      hitPriority = 0;
      Warn(player, "specified negative hit priority");
    }
    hitDefParams->hitPriority = hitPriority;
  }
  hitDefParams->hitPriorityType = paramsEx->hitPriorityType;

  hitDefParams->attackWidthZ1 = EvalExpressionI(player, &(paramsEx->attackWidthZ1), 1);
  hitDefParams->attackWidthZ2 = EvalExpressionI(player, &(paramsEx->attackWidthZ2), 1);

  {
    int32_t p1StateNo = EvalExpressionI(player, &(paramsEx->p1StateNo), 1);
    if (p1StateNo < -1) {
      p1StateNo = -1;
      Warn(player, "specified negative p1stateno");
    }
    hitDefParams->p1StateNo = p1StateNo;
  }
  {
    int32_t p2StateNo = EvalExpressionI(player, &(paramsEx->p2StateNo), 1);
    if (p2StateNo < -1) {
      p2StateNo = -1;
      Warn(player, "specified negative p2stateno");
    }
    hitDefParams->p2StateNo = p2StateNo;
  }
  hitDefParams->p2GetP1StateNo = (EvalExpressionI(player, &(paramsEx->p2GetP1StateNo), 1) != 0);

  {
    int32_t p2BindTime = EvalExpressionI(player, &(paramsEx->p2BindTime), 1);
    if (p2BindTime < -1) {
      p2BindTime = 1;
      Warn(player, "specified illegal p2bindtime");
    }
    hitDefParams->p2BindTime = p2BindTime;
  }

  hitDefParams->p1SprPriority = EvalExpressionI(player, &(paramsEx->p1SprPriority), 1);
  hitDefParams->p2SprPriority = EvalExpressionI(player, &(paramsEx->p2SprPriority), 1);
  hitDefParams->animType = paramsEx->animType;
  hitDefParams->forceStand = (EvalExpressionI(player, &(paramsEx->forceStand), 1) != 0);
  hitDefParams->reversalAttr = paramsEx->reversalAttr;

  if (EvalExpressionI(player, &(paramsEx->fall), 1) != 0) {
    hitDefParams->hitFlag |= 0x1;
  }

  hitDefParams->fall_damage = EvalExpressionI(player, &(paramsEx->fall_damage), 1);
  hitDefParams->fall_animType = paramsEx->fall_animType;
  hitDefParams->fall_XVelocity = EvalExpressionF(player, &(paramsEx->fall_XVelocity));
  hitDefParams->fall_YVelocity = EvalExpressionF(player, &(paramsEx->fall_YVelocity));
  hitDefParams->fall_recover = (EvalExpressionI(player, &(paramsEx->fall_recover), 1) != 0);
  {
    int32_t fall_recover_time = EvalExpressionI(player, &(paramsEx->fall_recover_time), 1);
    if (fall_recover_time < 0) {
      fall_recover_time = 4;
      Warn(player, "specified negative recovertime");
    }
    hitDefParams->fall_recover_time = fall_recover_time;
  }

  hitDefParams->sSparkFlag = paramsEx->sSparkFlag;
  {
    int32_t sparkNo = EvalExpressionI(player, &(paramsEx->sparkNo), 1);
    if (sparkNo < -2) {
      sparkNo = -2;
      Warn(player, "illegal spark no");
    }
    hitDefParams->sparkNo = sparkNo;
  }

  hitDefParams->guard_sSparkFlag = paramsEx->guard_sSparkFlag;
  {
    int32_t guard_sparkNo = EvalExpressionI(player, &(paramsEx->guard_sparkNo), 1);
    if (guard_sparkNo < -2) {
      guard_sparkNo = -2;
      Warn(player, "illegal gspark no");
    }
    hitDefParams->guard_sparkNo = guard_sparkNo;
  }

  hitDefParams->sparkX = EvalExpressionI(player, &(paramsEx->sparkX), 1);
  hitDefParams->sparkY = EvalExpressionI(player, &(paramsEx->sparkY), 1);

  hitDefParams->p1Facing = (EvalExpressionI(player, &(paramsEx->p1Facing), 1) < 0);
  {
    int32_t p1GetP2Facing = EvalExpressionI(player, &(paramsEx->p1GetP2Facing), 1);
    if (p1GetP2Facing != 0) {
      hitDefParams->p1Facing = 0;
      hitDefParams->p1GetP2Facing = (p1GetP2Facing > 0 ? 1 : -1);
    }
  }

  hitDefParams->minDistX = EvalExpressionI(player, &(paramsEx->minDistX), 1);
  hitDefParams->minDistY = EvalExpressionI(player, &(paramsEx->minDistY), 1);
  hitDefParams->maxDistX = EvalExpressionI(player, &(paramsEx->maxDistX), 1);
  hitDefParams->maxDistY = EvalExpressionI(player, &(paramsEx->maxDistY), 1);
  // minDistZ, maxDistZ は評価されない

  {
    int32_t p2Facing = EvalExpressionI(player, &(paramsEx->p2Facing), 1);
    if (p2Facing < -1 || p2Facing > 1) {
      p2Facing = 0;
      Warn(player, "illegal p2facing parameter");
    }
    hitDefParams->p2Facing = 0;
  }

  hitDefParams->ground_type = paramsEx->ground_type;
  {
    int32_t ground_hitTime = EvalExpressionI(player, &(paramsEx->ground_hitTime), 1);
    if (ground_hitTime < 0) {
      ground_hitTime = 0;
      Warn(player, "negative ground.p2hittime");
    }
    hitDefParams->ground_hitTime = ground_hitTime;
  }
  {
    int32_t ground_slideTime = EvalExpressionI(player, &(paramsEx->ground_slideTime), 1);
    if (ground_slideTime < 0) {
      ground_slideTime = 0;
      Warn(player, "negative ground.p2slidetime");
    }
    hitDefParams->ground_slideTime = ground_slideTime;
  }
  hitDefParams->ground_velocityX = EvalExpressionF(player, &(paramsEx->ground_velocityX));
  {
    float ground_velocityY = EvalExpressionF(player, &(paramsEx->ground_velocityY));
    hitDefParams->ground_velocityY = ground_velocityY;
    if (ground_velocityY != 0.0F && paramsEx->hasForceStand == 0) {
      hitDefParams->forceStand = 1;
    }
  }
  hitDefParams->ground_cornerPush_velOff = EvalExpressionF(player, &(paramsEx->ground_cornerPush_velOff));

  hitDefParams->air_type = paramsEx->air_type;
  hitDefParams->air_animType = paramsEx->air_animType;
  {
    int32_t air_hitTime = EvalExpressionI(player, &(paramsEx->air_hitTime), 1);
    if (air_hitTime < 0) {
      air_hitTime = 0;
      Warn(player, "negative air.p2hittime");
    }
    hitDefParams->air_hitTime = air_hitTime;
  }
  hitDefParams->air_fall = (uint32_t)EvalExpressionF(player, &(paramsEx->air_fall));
  hitDefParams->air_velocityX = EvalExpressionF(player, &(paramsEx->air_velocityX));
  hitDefParams->air_velocityY = EvalExpressionF(player, &(paramsEx->air_velocityY));
  hitDefParams->air_cornerPush_velOff = EvalExpressionF(player, &(paramsEx->air_cornerPush_velOff));

  hitDefParams->down_bounce = EvalExpressionI(player, &(paramsEx->down_bounce), 1);
  {
    int32_t down_hitTime = EvalExpressionI(player, &(paramsEx->down_hitTime), 1);
    if (down_hitTime < 0) {
      down_hitTime = 0;
      Warn(player, "negative down.p2hittime");
    }
    hitDefParams->down_hitTime = down_hitTime;
  }
  hitDefParams->down_velocityX = EvalExpressionF(player, &(paramsEx->down_velocityX));
  hitDefParams->down_velocityY = EvalExpressionF(player, &(paramsEx->down_velocityY));
  hitDefParams->down_cornerPush_velOff = EvalExpressionF(player, &(paramsEx->down_cornerPush_velOff));

  hitDefParams->guard_velocity = EvalExpressionF(player, &(paramsEx->guard_velocity));
  {
    int32_t guard_hitTime = EvalExpressionI(player, &(paramsEx->guard_hitTime), 1);
    if (guard_hitTime < 0) {
      guard_hitTime = 0;
      Warn(player, "negative guard.p2hittime");
    }
    hitDefParams->guard_hitTime = guard_hitTime;
  }
  {
    int32_t guard_slideTime = EvalExpressionI(player, &(paramsEx->guard_slideTime), 1);
    if (guard_slideTime < 0) {
      guard_slideTime = 0;
      Warn(player, "negative guard.p2slidetime");
    }
    hitDefParams->guard_slideTime = guard_slideTime;
  }
  {
    int32_t guard_ctrlTime = EvalExpressionI(player, &(paramsEx->guard_ctrlTime), 1);
    if (guard_ctrlTime < 0) {
      guard_ctrlTime = 0;
      Warn(player, "negative guard.p2ctrltime");
    }
    hitDefParams->guard_ctrlTime = guard_ctrlTime;
  }
  hitDefParams->guard_dist = EvalExpressionI(player, &(paramsEx->guard_dist), 1);
  {
    int32_t guard_p2PauseTime = EvalExpressionI(player, &(paramsEx->guard_p2PauseTime), 1);
    if (guard_p2PauseTime < 0) {
      guard_p2PauseTime = 0;
      Warn(player, "negative guard.p2shaketime");
    }
    hitDefParams->guard_p2PauseTime = guard_p2PauseTime;
  }
  {
    int32_t guard_p1PauseTime = EvalExpressionI(player, &(paramsEx->guard_p1PauseTime), 1);
    if (guard_p1PauseTime < 0) {
      guard_p1PauseTime = 0;
      Warn(player, "negative guard.p1pausetime");
    }
    hitDefParams->guard_p1PauseTime = guard_p1PauseTime;
  }
  hitDefParams->guard_cornerPush_velOff = EvalExpressionF(player, &(paramsEx->guard_cornerPush_velOff));

  hitDefParams->airGuard_velocityX = EvalExpressionF(player, &(paramsEx->airGuard_velocityX));
  hitDefParams->airGuard_velocityY = EvalExpressionF(player, &(paramsEx->airGuard_velocityY));
  {
    int32_t airGuard_ctrlTime = EvalExpressionI(player, &(paramsEx->airGuard_ctrlTime), 1);
    if (airGuard_ctrlTime < 0) {
      airGuard_ctrlTime = 0;
      Warn(player, "negative airguard.p2ctrltime");
    }
    hitDefParams->airGuard_ctrlTime = airGuard_ctrlTime;
  }
  hitDefParams->guard_cornerPush_velOff = EvalExpressionF(player, &(paramsEx->guard_cornerPush_velOff));

  hitDefParams->YAccel = EvalExpressionF(player, &(paramsEx->YAccel));

  SCtrlInitPalFX(&(paramsEx->palFX), &(hitDefParams->palFX), player);

  // 下記の二つのSCtrlInitEnvShake相当の処理はバイナリではインライン化されている
  SCtrlInitEnvShake(&(paramsEx->envShake), &(hitDefParams->envShake), player);
  (hitDefParams->envShake).elapsedTime = 0;
  SCtrlInitEnvShake(&(paramsEx->fall_envShake), &(hitDefParams->fall_envShake), player);
  (hitDefParams->fall_envShake).elapsedTime = 0;

  return 1;
}

int32_t R_SCtrlInitProjectile(MUGEN_SC_PARAMS_EX_PROJECTILE* paramsEx, MUGEN_PLAYER* player, MUGEN_PLAYER* enemyFirst, MUGEN_PLAYER* owner) {
  MUGEN_PROJECTILE_DATA projData;
  SecureZeroMemory(&projData, sizeof(projData));
  projData.projId = EvalExpressionI(player, &(paramsEx->projId), 1);
  if (projData.projId < 0) {
    projData.projId = 0;
    Warn(player, "negative projectile ID");
  }
  projData.teamSide = player->teamSide;
  projData.projRemove = (EvalExpressionI(player, &(paramsEx->projRemove), 1) != 0);
  projData.projRemoveTime = EvalExpressionI(player, &(paramsEx->projRemoveTime), 1);
  if (projData.projRemoveTime < -1) {
    projData.projRemoveTime = 1;
    Warn(player, "illegal projremovetime");
  }
  {
    int32_t shadowR = EvalExpressionI(player, &(paramsEx->projShadow_R), 1);
    int32_t shadowG = EvalExpressionI(player, &(paramsEx->projShadow_G), 1);
    int32_t shadowB = EvalExpressionI(player, &(paramsEx->projShadow_B), 1);
    if (shadowR < -1 || shadowG < -1 || shadowB < -1 || shadowR > 255 || shadowG > 255 || shadowB > 255) {
      Warn(player, "Illegal shadow color");
      projData.projShadow = 0xffffffff;
    } else if (shadowR == -1 || shadowG == -1 || shadowB == -1) {
      projData.projShadow = 0xffffffff;
    } else {
      projData.projShadow = ((shadowB >> 3) << (_rgb_b_shift_16 & 0x1F)) | ((shadowG >> 2) << (_rgb_g_shift_16 & 0x1F)) | ((shadowR >> 3) << (_rgb_r_shift_16 & 0x1F));
    }
  }
  projData.projMissTime = EvalExpressionI(player, &(paramsEx->projMissTime), 1);
  if (projData.projMissTime < 0) {
    projData.projMissTime = 2;
    Warn(player, "negative projectile miss time");
  }
  projData.projHits = EvalExpressionI(player, &(paramsEx->projHits), 1);
  if (projData.projHits < 0) {
    projData.projHits = 1;
    Warn(player, "negative # of projectile hits");
  }
  projData.projPriority = EvalExpressionI(player, &(paramsEx->projPriority), 1);
  projData.projHitAnim = EvalExpressionI(player, &(paramsEx->projHitAnim), 1);
  if (projData.projHitAnim < -1) {
    projData.projHitAnim = -1;
    Warn(player, "illegal projhit actionid");
  }
  projData.projRemAnim = EvalExpressionI(player, &(paramsEx->projRemAnim), 1);
  if (projData.projRemAnim < -1) {
    Warn(player, "illegal projremove actionid");
  }
  projData.projCancelAnim = EvalExpressionI(player, &(paramsEx->projCancelAnim), 1);
  if (projData.projCancelAnim < -1) {
    Warn(player, "illegal projcancel actionid");
  }
  projData.velocityX = EvalExpressionF(player, &(paramsEx->velocityX));
  projData.velocityY = EvalExpressionF(player, &(paramsEx->velocityY));
  projData.velMulX = EvalExpressionF(player, &(paramsEx->velMulX));
  projData.velMulY = EvalExpressionF(player, &(paramsEx->velMulY));
  projData.remVelocityX = EvalExpressionF(player, &(paramsEx->remVelocityX));
  projData.remVelocityY = EvalExpressionF(player, &(paramsEx->remVelocityY));
  projData.accelX = EvalExpressionF(player, &(paramsEx->accelX));
  projData.accelY = EvalExpressionF(player, &(paramsEx->accelY));
  projData.projScaleX = EvalExpressionF(player, &(paramsEx->projScaleX));
  projData.projScaleY = EvalExpressionF(player, &(paramsEx->projScaleY));
  if ((player->consts).size_proj_doScale != 0) {
    projData.projScaleX *= (player->consts).size_XScale;
    projData.projScaleY *= (player->consts).size_YScale;
  }
  SCtrlInitHitDef(&(paramsEx->hitDefParams), &(projData.projHitDefParams), player);
  projData.projHitDefParams.attackNo = (g->nextAttackNo)++;
  if (projData.projHitDefParams.attr != 0) {
    projData.projHitDefParams.isAttacking = 1;
  }
  if (projData.projHitDefParams.guard_dist == -1) {
    projData.projHitDefParams.guard_dist = (player->consts).size_proj_attack_dist;
  }
  projData.facing = player->facing;
  projData.projBindTime = 0;
  projData.time = 0;
  projData.elapsedTime = 0;
  projData.status = 1;
  {
    int32_t basePosX;
    int32_t basePosY;
    switch (paramsEx->posType) {
      case 1:
        basePosX = g->iStageAxisX + (projData.facing == -1 ? 0 : 320);
        basePosY = 0;
        break;
      case 2:
        basePosX = g->iStageAxisX + (projData.facing == 1 ? 0 : 320);
        basePosY = 0;
        break;
      case 3:
        basePosX = g->iStageAxisX;
        basePosY = 0;
        break;
      case 4:
        basePosX = g->iStageAxisX + 320;
        basePosY = 0;
        break;
      case 5:
        if (enemyFirst != NULL) {
          basePosX = (int32_t)(enemyFirst->screenPosX);
          basePosY = (int32_t)(enemyFirst->posY);
          break;
        }
        // fall through
      case 0:
        basePosX = (int32_t)(player->screenPosX);
        basePosY = (int32_t)(player->posY);
        break;
    }
    projData.posX = (float)(EvalExpressionI(player, &(paramsEx->posX), 0) * player->facing + basePosX);
    projData.posY = (float)(EvalExpressionI(player, &(paramsEx->posY), 0) + basePosY);
    projData.posZ = player->posZ;
  }
  projData.velocityX *= player->facing;
  projData.remVelocityX *= player->facing;
  projData.accelX *= player->facing;
  projData.projSprPriority = EvalExpressionI(player, &(paramsEx->projSprPriority), 1);
  {
    int32_t projStageBound = EvalExpressionI(player, &(paramsEx->projStageBound), 0);
    projData.projStageBoundLeft = (g->camera).boundLeft - projStageBound;
    projData.projStageBoundRight = (g->camera).boundRight + 320 + projStageBound;
  }
  projData.projEdgeBound = EvalExpressionI(player, &(paramsEx->projEdgeBound), 0);
  projData.projHeightBoundUpper = EvalExpressionI(player, &(paramsEx->projHeightBoundUpper), 0);
  projData.projHeightBoundBottom = EvalExpressionI(player, &(paramsEx->projHeightBoundBottom), 0);

  int32_t projAnim = EvalExpressionI(player, &(paramsEx->projAnim), 1);
  if (projAnim < -1) {
    projAnim = 0;
    Warn(player, "illegal projanim");
  }

  /* ここだけfloatをintにキャストしている */
  projData.superMoveTime = (int32_t)EvalExpressionF(player, &(paramsEx->superMoveTime));
  projData.pauseMoveTime = (int32_t)EvalExpressionF(player, &(paramsEx->pauseMoveTime));
  int32_t index = ProjArrayAdd(owner, owner->projectiles, &projData, projAnim);
  if (index != -1) {
    if (index < 0 || index >= owner->projectiles->projInfo->maxProjCnt) {
      sprintf((char*)DebugTmpStr, "Assert failure in array.h line 110");
      DebugErrorExit((char*)DebugTmpStr);
    }
    MUGEN_PROJECTILE_DATA* data = &((owner->projectiles->projInfo)->data[index]);
    if (paramsEx->afterImage != NULL) {
      if (data->afterImage == NULL) {
        data->afterImage = RequestNewAftImg(g->afterImages);
      }
      if (data->afterImage != NULL) {
        SCtrlInitAftImgParams(paramsEx->afterImage, data->afterImage, data->anim, player);
      }
    }
  }
  return index;
}

static inline void SCtrlClipboard_(MUGEN_PLAYER* player, MUGEN_SC_ID scId, char* text, ...) __attribute__((nonnull(1, 3)));

static inline void SCtrlClipboard(MUGEN_PLAYER* player, MUGEN_SC_ID scId, int32_t argc, char* text, MUGEN_EVAL_VALUE* params) {
  // int型とdouble型をないまぜにした配列にparamsの各評価値を詰め、
  // int, float混じりの動的引数呼び出しを実現
  size_t ptr = 0;
  int32_t buf[10] = {0};
  if (argc >= 1) {
    int32_t intResult;
    float floatResult;
    if (EvalExpression(player, &(params[0]), &intResult, &floatResult) == 2) {
      *(double*)(buf + ptr) = (double)floatResult;
      ptr += 2;
    } else {
      buf[ptr] = intResult;
      ptr += 1;
    }
    if (argc >= 2) {
      if (EvalExpression(player, &(params[1]), &intResult, &floatResult) == 2) {
        *(double*)(buf + ptr) = (double)floatResult;
        ptr += 2;
      } else {
        buf[ptr] = intResult;
        ptr += 1;
      }
      if (argc >= 3) {
        if (EvalExpression(player, &(params[2]), &intResult, &floatResult) == 2) {
          *(double*)(buf + ptr) = (double)floatResult;
          ptr += 2;
        } else {
          buf[ptr] = intResult;
          ptr += 1;
        }
        if (argc >= 4) {
          if (EvalExpression(player, &(params[3]), &intResult, &floatResult) == 2) {
            *(double*)(buf + ptr) = (double)floatResult;
            ptr += 2;
          } else {
            buf[ptr] = intResult;
            ptr += 1;
          }

          if (argc >= 5) {
            if (EvalExpression(player, &(params[4]), &intResult, &floatResult) == 2) {
              *(double*)(buf + ptr) = (double)floatResult;
              ptr += 2;
            } else {
              buf[ptr] = intResult;
              ptr += 1;
            }
          }
        }
      }
    }

    SCtrlClipboard_(player, scId, text, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
  }
}

static inline void SCtrlClipboard_(MUGEN_PLAYER* player, MUGEN_SC_ID scId, char* text, ...) {
  char temp[1024];
  va_list ap;
  va_start(ap, text);
  // J@K式, 4CA式利用時（かつ%1024dのように%が一つ場合）, このvsprintf関数で最初のスタックオーバーフローが発生する
  // ただし, ここでオーバーフローする場合, 必ずClipboardAppend, ClipboardPrint関数内でも
  // 同様にスタックオーバーフローが発生し, 先に後者の影響が表れる
  vsprintf(temp, text, ap);
  va_end(ap);
  if (scId == ID_SC_AppendToClipboard) {
    ClipboardAppend(player->clipboard, temp);
  } else {
    ClipboardPrint(player->clipboard, temp);
  }
}
