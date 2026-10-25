/**
 * @file SCtrlTrigEval.c
 * @author Halkaze
 * @brief SCtrlTrigEval（トリガー処理）関連関数群
 * @date 2023-03-17
 * @copyright Copyright (c) 2023
 *
 * 留意事項
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

#include <stdbool.h>

uint32_t R_SCtrlTrigArrayTest(MUGEN_PLAYER* player, MUGEN_TRIGGER_INFO* triggers, int32_t triggerCnt) __attribute__((nonnull(1, 2)));
void R_SCtrlTrigEval(MUGEN_PLAYER* player, MUGEN_EVAL_TRIGGER* trig) __attribute__((nonnull(1, 2)));

uint32_t R_SCtrlTrigArrayTest(MUGEN_PLAYER* player, MUGEN_TRIGGER_INFO* triggers, int32_t triggerCnt) {
  MUGEN_TRIGGER_DATA* data;
  int32_t cnt = 0;
  while ((data = (MUGEN_TRIGGER_DATA*)ArrayAccessGrpItem((ABSTRUCT_ARRAY_INFO*)triggers, -1, cnt++)) != NULL) {
    int32_t intResult;
    float floatResult;
    int32_t result = EvalExpression(player, &(data->value), &intResult, &floatResult);
    if (result == 0) {
      sprintf((char*)ErrorMessage, "%4i: : Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
      ErrorExit();
    } else if (result == 1) {
      if (intResult == 0) {
        return 0;
      }
    } else if (floatResult == 0.0F) {
      return 0;
    }
  }
  for (int32_t i = 0; i < triggerCnt; ++i) {
    data = (MUGEN_TRIGGER_DATA*)ArrayAccessGrpItem((ABSTRUCT_ARRAY_INFO*)triggers, i, 0);
    if (data == NULL) {
      strcpy((char*)(g->errorMessage), "TrigArrayTest bug");
      ErrorExit();
    }
    int32_t j = 0;
    while (true) {
      int32_t intResult;
      float floatResult;
      int32_t result = EvalExpression(player, &(data->value), &intResult, &floatResult);
      if (result == 0) {
        sprintf((char*)ErrorMessage, "%4i: Fatal: Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
        ErrorExit();
      } else if (result == 1) {
        if (intResult == 0) {
          break;
        }
      } else if (floatResult == 0.0F) {
        break;
      }
      ++j;
      data = (MUGEN_TRIGGER_DATA*)ArrayAccessGrpItem((ABSTRUCT_ARRAY_INFO*)triggers, i, j);
      if (data == NULL) {
        return 1;
      }
    }
  }
  return 0;
}

#define PushMyStackI(data) PushMyStack((int32_t)(data))
#define PushMyStackF(data) PushMyStack((float)(data))
#define PushMyStackT(data) PushMyStack((MUGEN_EVAL_TYPE)(data))
#define PushMyStack(data) PushMyStack_((MUGEN_EVAL_NODE)(data))
static inline void PushMyStack_(MUGEN_EVAL_NODE data);
static inline MUGEN_EVAL_NODE PopMyStack(void);

void R_StackUnderflow(void);
uint32_t R_SPopI(MUGEN_PLAYER* player, int32_t* pInt) __attribute__((nonnull(1, 2)));
uint32_t R_SPopF(__attribute__((unused)) MUGEN_PLAYER* player, float* pFloat) __attribute__((nonnull(1)));

static inline void PushMyStack_(MUGEN_EVAL_NODE data) {
  if (MyStackIndex > 126) {
    sprintf((char*)ErrorMessage, "Expression stack overflow.");
    ErrorExit();
  } else {
    MyStack[++MyStackIndex] = data;
  }
}

static inline MUGEN_EVAL_NODE PopMyStack(void) {
  if (MyStackIndex < 0) {
    StackUnderflow();
  }
  return MyStack[MyStackIndex--];
}

void R_StackUnderflow(void) {
  sprintf((char*)ErrorMessage, "Fatal: Expression stack underflow.");
  ErrorExit();
}

uint32_t R_SPopI(MUGEN_PLAYER* player, int32_t* pInt) {
  MUGEN_EVAL_TYPE a_type = (PopMyStack()).t;
  MUGEN_EVAL_NODE a = PopMyStack();
  if (a_type == EVAL_TYPE_FLOAT) {
    *pInt = (int32_t)(a.f);
    Warn(player, "trigger arg truncated to int");
    return 2;
  } else {
    *pInt = a.i;
    return (a_type == EVAL_TYPE_ERROR ? 3 : 2);
  }
}

uint32_t R_SPopF(__attribute__((unused)) MUGEN_PLAYER* player, float* pFloat) {
  MUGEN_EVAL_TYPE a_type = (PopMyStack()).t;
  MUGEN_EVAL_NODE a = PopMyStack();
  if (a_type == EVAL_TYPE_FLOAT) {
    *pFloat = a.f;
    return 2;
  } else {
    *pFloat = (float)(a.i);
    return (a_type == EVAL_TYPE_ERROR ? 3 : 2);
  }
}

static inline int32_t GetTrigArgc(MUGEN_TRIG_ID id);
static inline int32_t GetTrigArgc(MUGEN_TRIG_ID id) {
  if (id >= 0x140 && id <= 0x15C) {
    return 1;
  } else if (id >= 0x136 && id <= 0x136) {
    return 2;
  } else if (id >= 0x131 && id <= 0x131) {
    return 3;
  } else {
    return 0;
  }
}

static inline bool HasCompTrig(MUGEN_TRIG_ID id);
static inline bool HasCompTrig(MUGEN_TRIG_ID id) {
  switch (id) {
    case ID_TRIG_Command:
    case ID_TRIG_AnimElem:
    case ID_TRIG_Name:
    case ID_TRIG_P2Name:
    case ID_TRIG_P3Name:
    case ID_TRIG_P4Name:
    case ID_TRIG_StateType:
    case ID_TRIG_P2StateType:
    case ID_TRIG_MoveType:
    case ID_TRIG_P2MoveType:
    case ID_TRIG_TimeMod:
    case ID_TRIG_ProjHit:
    case ID_TRIG_ProjContact:
    case ID_TRIG_ProjGuarded:
      return true;
    default:
      return false;
  }
}

void R_SCtrlTrigEval(MUGEN_PLAYER* player, MUGEN_EVAL_TRIGGER* trig) {
  MUGEN_PLAYER* redirect_player;
  MUGEN_PLAYER* p1 = player;
  MUGEN_PLAYER* p2 = NULL;
  MUGEN_PLAYER* p3 = NULL;
  MUGEN_PLAYER* p4 = NULL;
  MUGEN_PLAYER* team_first = player;
  int32_t result_inversed = 0;

  if (trig->redirectID == ID_RD_Parent) {
    redirect_player = p1->parent;
    if (p1->parent == NULL) {
      Warn(p1, "has no parent");
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
  } else if (trig->redirectID == ID_RD_Root) {
    redirect_player = p1->root;
    if (p1->root == NULL) {
      Warn(p1, "has no root");
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
  } else if (trig->redirectID == ID_RD_Helper) {
    int32_t helperID = EvalExpressionI(p1, &(trig->redirectArg), 1);
    if (helperID < 0) {
      helperID = 0;
      Warn(p1, "Illegal helper ID in retargeted trigger");
    }
    redirect_player = HelperAccessID(p1, helperID);
    if (redirect_player == NULL) {
      VWarn(p1, "has no helper %i", helperID);
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
  } else if (trig->redirectID == ID_RD_Target) {
    int32_t targetID = EvalExpressionI(p1, &(trig->redirectArg), 1);
    if (targetID < -1) {
      targetID = -1;
      Warn(p1, "Illegal target ID in retargeted trigger");
    }
    MUGEN_TARGET_DATA* data = TargetAccessID(p1->targets, targetID);
    if (data == NULL) {
      if ((int32_t)(player) == -1) {
        // "targetID == -1"のミス？, targetIDの指定によらずVWarnしか実行されない
        Warn(p1, "has no target");
      } else {
        VWarn(p1, "has no target with hit ID %i", targetID);
      }
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
    redirect_player = data->player;
  } else if (trig->redirectID == ID_RD_Partner) {
    int32_t no = EvalExpressionI(p1, &(trig->redirectArg), 1);
    if (no < 0) {
      no = 0;
      Warn(p1, "Illegal partner number in retargeted trigger");
    }
    redirect_player = PartnerAccess(p1, no);
    if (redirect_player == NULL) {
      VWarn(p1, "has no %i-th partner", no);
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
  } else if (trig->redirectID == ID_RD_Enemy) {
    int32_t no = EvalExpressionI(p1, &(trig->redirectArg), 1);
    if (no < 0) {
      no = 0;
      Warn(p1, "Illegal enemy number in retargeted trigger");
    }
    redirect_player = EnemyAccess(p1, no);
    if (redirect_player == NULL) {
      VWarn(p1, "has no %i-th enemy", no);
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
  } else if (trig->redirectID == ID_RD_EnemyNear) {
    int32_t no = EvalExpressionI(p1, &(trig->redirectArg), 1);
    if (no < 0) {
      no = 0;
      Warn(p1, "Illegal near enemy number in retargeted trigger");
    }
    redirect_player = EnemyAccessNear(p1, no);
    if (redirect_player == NULL) {
      VWarn(p1, "has no %i-th nearest enemy", no);
      MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
      PushMyStackI(0);
      PushMyStackT(EVAL_TYPE_ERROR);
      return;
    }
  } else {
    redirect_player = p1;
    if (trig->redirectID == ID_RD_PlayerID) {
      int32_t playerID = EvalExpressionI(p1, &(trig->redirectArg), 1);
      if (playerID < 0) {
        playerID = 0;
        Warn(p1, "Illegal player number in retargeted trigger");
      }
      redirect_player = PlayerAccessID(playerID);
      if (redirect_player == NULL) {
        VWarn(p1, "tried to access non-existent player with ID %i", playerID);
        MyStackIndex = MyStackIndex - 2 * GetTrigArgc(trig->trigID);
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
    }
  }

  MUGEN_PLAYER* redirect_parent = HelperParent(redirect_player);
  MUGEN_PLAYER* redirect_root = HelperRoot(redirect_player);
  MUGEN_PLAYER* furthestEnemy = NULL;
  uint8_t useRedirect = (redirect_player != p1);
  if ((g->rules).gameType == GAMETYPE_VS) {
    MUGEN_PLAYER* enemy1 = NULL;
    MUGEN_PLAYER* enemy2 = NULL;
    if (redirect_player->teamSide == 1) {
      enemy1 = g->playerList[2];
      enemy2 = g->playerList[4];
    } else if (redirect_player->teamSide == 2) {
      enemy1 = g->playerList[1];
      enemy2 = g->playerList[3];
    }
    p2 = redirect_player->p2Enemy;
    if (enemy1 == NULL || enemy2 == NULL) {
      furthestEnemy = p4;
      if (p2 != NULL) {
        if (p2->isHelper == 1) {
          p4 = enemy1;
          furthestEnemy = p4;
          if (enemy1 == HelperRoot(p2)) {
            furthestEnemy = enemy2;
          }
        } else {
          furthestEnemy = enemy1;
          if (p2 == enemy1) {
            furthestEnemy = enemy2;
          }
        }
      }
    } else {
      furthestEnemy = enemy2;
      if (p2 != enemy1) {
        furthestEnemy = enemy1;
        if (p2 != enemy2) {
          if (__builtin_abs(PlayerDistX(redirect_player, enemy2)) > __builtin_abs(PlayerDistX(redirect_player, enemy1))) {
            p4 = enemy2;
            furthestEnemy = p4;
          }
        }
      }
    }
    p4 = furthestEnemy;
    if (redirect_player->isHelper == 1) {
      if (redirect_root != NULL) {
        int32_t slotID = redirect_root->slotId + 2;
        if (slotID > 4) {
          slotID -= 2;
        }
        p3 = g->playerList[slotID];
      }
      if (redirect_player->teamSide == 1) {
        p3 = g->playerList[1];
        if (p3 == NULL) {
          p3 = g->playerList[3];
        }
      } else if (redirect_player->teamSide == 2) {
        p3 = g->playerList[2];
        if (p3 == NULL) {
          p3 = g->playerList[4];
        }
      }
    } else {
      int32_t slotID = redirect_player->slotId + 2;
      if (slotID > 4) {
        slotID -= 2;
      }
      p3 = g->playerList[slotID];
    }
    if (redirect_player->teamSide == 1) {
      team_first = g->playerList[1];
    }
    if (redirect_player->teamSide == 2) {
      team_first = g->playerList[2];
    }
  }

  MUGEN_EVAL_NODE result;
  switch (trig->trigID) {
    case ID_TRIG_Time:  // Time / StateTime
      result.i = redirect_player->time;
      break;
    case ID_TRIG_AnimTime: {  // AnimTime
      int32_t temp = AnimGetLooptime(redirect_player->anim);
      result.i = redirect_player->anim->currentFrame - temp;
    } break;
    case ID_TRIG_AnimElem: {  // AnimElem = $2, $cmp $0
      int32_t temp = AnimCurElemStartTime(redirect_player->anim, (trig->trigArgs)[2].i);
      if (temp == -1) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = redirect_player->anim->currentFrame - temp;
    } break;
    case ID_TRIG_Anim:  // Anim
      result.i = AnimCurIDno(redirect_player->anim);
      break;
    case ID_TRIG_Pos:  // Pos X / Pos Y / Pos Z, $0で判別
      if ((trig->trigArgs[0]).i == 0) {
        // Pos X
        result.f = (redirect_player->screenPosX - g->fStageAxisX) - 160.0F;
      } else if ((trig->trigArgs)[0].i == 1) {
        // Pos Y
        result.f = redirect_player->posY;
      } else if ((trig->trigArgs)[0].i == 2) {
        // Pos Z
        result.f = redirect_player->posZ;
      }
      break;
    case ID_TRIG_ScreenPos:  // ScreenPos X / ScreenPos Y / ScreenPos Z
                             // $0で判別
      if ((trig->trigArgs)[0].i == 0) {
        // ScreenPos X
        result.f = redirect_player->screenPosX - g->fStageAxisX;
      } else if ((trig->trigArgs)[0].i == 1) {
        // ScreenPos Y
        result.f = (redirect_player->posY - g->fStageAxisY) + g->zOffset;
      } else if ((trig->trigArgs)[0].i == 2) {
        // ScreenPos Z
        result.f = redirect_player->posZ;
      }
      break;
    case ID_TRIG_Vel:  // Vel X / Vel Y / Vel Z, $0で判別
      if ((trig->trigArgs)[0].i == 0) {
        // Vel X
        result.f = redirect_player->facing * redirect_player->velX;
      } else if ((trig->trigArgs)[0].i == 1) {
        // Vel Y
        result.f = redirect_player->velY;
      } else if ((trig->trigArgs)[0].i == 2) {
        // Vel Z
        result.f = redirect_player->velZ;
      }
      break;
    case ID_TRIG_TimeMod:  // TimeMod $cmp $2, $0
      result.i = (redirect_player->time) % ((trig->trigArgs)[2].i);
      break;
    case ID_TRIG_P2Dist:  // P2Dist X / P2Dist Y / P2Dist Z, $0で判別
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      if ((trig->trigArgs)[0].i == 0) {
        // P2Dist X
        result.f = (float)PlayerDistX(redirect_player, p2);
      } else if ((trig->trigArgs)[0].i == 1) {
        // P2Dist Y
        result.f = p2->posY - redirect_player->posY;
      } else if ((trig->trigArgs)[0].i == 2) {
        // p2Dist Z
        result.f = p2->posZ - redirect_player->posZ;
      }
      break;
    case ID_TRIG_P2BodyDist:  // P2BodyDist X / P2BodyDist Y / P2BodyDist Z
                              // $0で判別
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      if ((trig->trigArgs)[0].i == 0) {
        // P2BodyDist X
        result.f = (float)PlayerBodyDistX(redirect_player, p2);
      } else if ((trig->trigArgs)[0].i == 1) {
        // P2BodyDist Y
        result.f = p2->posY - redirect_player->posY;
      } else if ((trig->trigArgs)[0].i == 2) {
        // p2BodyDist Z
        result.f = p2->posZ - redirect_player->posZ;
      }
      break;
    case ID_TRIG_P2Life:  // P2Life
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = p2->life;
      break;
    case ID_TRIG_FrondEdgeDist:       // FrontEdgeDist / FrontEdgeBodyDist, $2で判別
    case ID_TRIG_BackEdgeBodyDist: {  // BackEdgeDist / BackEdgeBodyDist, 同上
      redirect_player->leftBounds = (int32_t)((float)((g->bound).screenRight - 160) / redirect_player->scale + (float)(g->iStageAxisX) + 160.0F);
      redirect_player->rightBounds = (int32_t)((160 - (g->bound).screenLeft) / redirect_player->scale + g->iStageAxisX + 160.0F);
      int32_t temp = 0;
      if ((trig->trigArgs)[2].i != 0) {
        if (redirect_player->stateType == STATETYPE_A) {
          temp = 1;
        } else if (redirect_player->stateType == STATETYPE_L) {
          temp = 2;
        }
      }
      if (trig->trigID == ID_TRIG_FrondEdgeDist) {
        if ((trig->trigArgs)[2].i != 0) {
          temp += redirect_player->widthEdgeFront;
        }
        int32_t temp2 = ((redirect_player->facing == 1) ? (redirect_player->rightBounds) : (redirect_player->leftBounds));
        result.i = (int32_t)((((redirect_player->facing * temp) + redirect_player->screenPosX) - temp2) * -(redirect_player->facing));
      } else {
        if ((trig->trigArgs)[2].i != 0) {
          temp += redirect_player->widthEdgeBack;
        }
        int32_t temp2 = ((redirect_player->facing == 1) ? (redirect_player->leftBounds) : (redirect_player->rightBounds));
        result.i = (int32_t)((((redirect_player->screenPosX - (redirect_player->facing * temp))) - temp2) * redirect_player->facing);
      }
    } break;
    case ID_TRIG_Life:  // Life
      result.i = redirect_player->life;
      break;
    case ID_TRIG_LifeMax:  // LifeMax
      result.i = redirect_player->lifeMax;
      break;
    case ID_TRIG_Power:  // Power
      result.i = *(team_first->pPower);
      break;
    case ID_TRIG_PowerMax:  // PowerMax
      result.i = *(team_first->pPowerMax);
      break;
    case ID_TRIG_Alive:  // Alive
      result.i = redirect_player->alive;
      break;
    case ID_TRIG_Command:  // Command $cmp $0
      result.i = (trig->trigArgs)[0].i - 1;
      if (KBCommandIsValid(&(redirect_player->commandTimes), (trig->trigArgs)[0].i) != 0) {
        result.i = (trig->trigArgs)[0].i;
        if (trig->compareStyle == EVAL_EQ) {
          AIInsertCommand((redirect_player->ai).commandHistory, (trig->trigArgs)[0].i);
        }
      }
      break;
    case ID_TRIG_HitCount:  // HitCount
      result.i = redirect_player->hitCount;
      break;
    case ID_TRIG_UniqHitCount:  // UniqHitCount
      result.i = redirect_player->uniqHitCount;
      break;
    case ID_TRIG_MoveHit:  // MoveHit
      if (redirect_player->moveContactType == MOVECONTACT_HIT) {
        result.i = redirect_player->moveContactTime + 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_MoveGuarded:  // MoveGuarded
      if (redirect_player->moveContactType == MOVECONTACT_GUARDED) {
        result.i = redirect_player->moveContactTime + 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_MoveContact:  // MoveContact
      if (redirect_player->moveContactType == MOVECONTACT_HIT || redirect_player->moveContactType == MOVECONTACT_GUARDED) {
        result.i = redirect_player->moveContactTime + 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_MoveReversed:  // MoveReversed
      if (redirect_player->moveContactType == MOVECONTACT_REVERSED) {
        result.i = redirect_player->moveContactTime + 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_InGuardDist:  // InGuardDist
      result.i = PlayerInAnyGuardDist(redirect_player);
      break;
    case ID_TRIG_StateNo:  // StateNo
      result.i = redirect_player->stateNo;
      break;
    case ID_TRIG_PrevStateNo:  // PrevStateNo
      result.i = redirect_player->prevStateNo;
      break;
    case ID_TRIG_P2StateNo:  // P2StateNo
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = p2->stateNo;
      break;
    case ID_TRIG_StateType:  // StateType $cmp $0
      result.i = redirect_player->stateType;
      break;
    case ID_TRIG_P2StateType:  // P2StateType $cmp $0
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = p2->stateType;
      break;
    case ID_TRIG_MoveType:  // MoveType $cmp $0
      result.i = redirect_player->moveType;
      break;
    case ID_TRIG_P2MoveType:  // P2MoveType $cmp $0
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = p2->moveType;
      break;
    case ID_TRIG_Ctrl:  // Ctrl
      result.i = redirect_player->ctrl;
      break;
    case ID_TRIG_HitDefAttr:  // HitDefAttr = $0
      result.i = 0;
      if (redirect_player->moveType == MOVETYPE_A) {
        uint32_t temp = ((redirect_player->hitDefParams).attr & (trig->trigArgs)[0].u);
        if ((temp & 0x7) != 0 && (temp & 0xfffffff8) != 0) {
          result.i = 1;
        }
      }
      break;
    case ID_TRIG_Random:  // Random
                          /* 独自の乱数生成関数に置換 */
      result.i = (uint32_t)rand() % 1000;
      break;
    case ID_TRIG_NumProj:  // NumProj
      result.i = ProjArrayCountActive(redirect_player->projectiles);
      break;
    case ID_TRIG_ProjHit:  // ProjHit($2) = $3, $cmp $0
      if (redirect_player->projContactType != PROJCONTACT_HIT || ((trig->trigArgs)[2].i != 0 && (trig->trigArgs)[2].i != redirect_player->projContactId)) {
        PushMyStackI((trig->trigArgs)[3].i == 0);
        PushMyStackT(EVAL_TYPE_INT);
        return;
      }
      result.i = redirect_player->projContactTime;
      if ((trig->trigArgs)[3].i == 0) {
        result_inversed = 1;
      }
      break;
    case ID_TRIG_ProjGuarded:  // ProjGuarded($2) = $3, $cmp $0
      if (redirect_player->projContactType != PROJCONTACT_GUARDED || ((trig->trigArgs)[2].i != 0 && (trig->trigArgs)[2].i != redirect_player->projContactId)) {
        PushMyStackI((trig->trigArgs)[3].i == 0);
        PushMyStackT(EVAL_TYPE_INT);
        return;
      }
      result.i = redirect_player->projContactTime;
      if ((trig->trigArgs)[3].i == 0) {
        result_inversed = 1;
      }
      break;
    case ID_TRIG_ProjContact:  // ProjContact($2) = $3, $cmp $0
      if (redirect_player->projContactType == PROJCONTACT_NONE || redirect_player->projContactType == PROJCONTACT_CANCEL || ((trig->trigArgs)[2].i != 0 && (trig->trigArgs)[2].i != redirect_player->projContactId)) {
        PushMyStackI((trig->trigArgs)[3].i == 0);
        PushMyStackT(EVAL_TYPE_INT);
        return;
      }
      result.i = redirect_player->projContactTime;
      if ((trig->trigArgs)[3].i == 0) {
        result_inversed = 1;
      }
      break;
    case ID_TRIG_Name:  // Name $cmp $str / P1Name $cmp $str
      result.i = (stricmp(redirect_player->info->name, trig->trigStrArg) == 0);
      break;
    case ID_TRIG_P2Name:  // P2Name $cmp $str
      if (p2 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = (stricmp(p2->info->name, trig->trigStrArg) == 0);
      break;
    case ID_TRIG_P3Name:  // P3Name $cmp $str
      if (p3 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = (stricmp(p3->info->name, trig->trigStrArg) == 0);
      break;
    case ID_TRIG_P4Name:  // P4Name $cmp $str
      if (p4 == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      result.i = (stricmp(p4->info->name, trig->trigStrArg) == 0);
      break;
    case ID_TRIG_AuthorName:  // AuthorName $cmp $str, ミスにより$cmpは常に=扱いとなっている
      result.i = (stricmp(redirect_player->info->authorName, trig->trigStrArg) == 0);
      break;
    case ID_TRIG_NumPartner:  // NumPartner
      result.i = PartnerCount(redirect_player);
      break;
    case ID_TRIG_NumEnemy:  // NumEnemy
      result.i = EnemyCount(redirect_player);
      break;
    case ID_TRIG_ID:  // ID
      result.i = redirect_player->playerId;
      break;
    case ID_TRIG_Facing:  // Facing
      result.i = redirect_player->facing;
      break;
    case ID_TRIG_HitFall:  // HitFall
      result.i = (redirect_player->getHitVars).fall;
      break;
    case ID_TRIG_HitShakeOver:  // HitShakeOver
      result.i = ((redirect_player->getHitVars).hitShakeTime < 1);
      break;
    case ID_TRIG_HitOver:  // HitOver
      result.i = ((redirect_player->getHitVars).hitTime < 0);
      break;
    case ID_TRIG_HitVel:  // HitVel X / HitVel Y, $0で判別
      if ((trig->trigArgs)[0].i == 0) {
        // HitVel X
        result.f = (redirect_player->facing * (redirect_player->getHitVars).XVel);
      } else if ((trig->trigArgs)[0].i == 1) {
        // HitVel Y
        result.f = (redirect_player->getHitVars).YVel;
      }
      break;
    case ID_TRIG_HitPauseTime:  // HitPauseTime
      result.i = redirect_player->hitPauseTime;
      break;
    case ID_TRIG_CanRecover:  // CanRecover
      if ((redirect_player->getHitVars).fall_recover == 0 || (redirect_player->getHitVars).fall_time < (redirect_player->getHitVars).fall_recoverTime) {
        result.i = 0;
      } else {
        result.i = 1;
      }
      break;
    case ID_TRIG_PalNo:  // PalNo
      result.i = redirect_player->palNo + 1;
      break;
    case ID_TRIG_WinPerfect:  // WinPerfect
      if (g->settledType != 0 && (g->winTeam == redirect_player->teamSide)) {
        if (TeamHasFullLife(redirect_player->teamSide) != 0) {
          result.i = 1;
          break;
        }
      }
      result.i = 0;
      break;
    case ID_TRIG_Win:  // Win / WinKO / WinTime, $2で判別
      if ((g->settledType == (trig->trigArgs)[2].u || (trig->trigArgs)[2].u == 0xFFFFFFFF) && g->winTeam == redirect_player->teamSide) {
        result.i = 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_Lose:  // Lose / LoseKO / LoseTime, $2で判別
      if ((g->settledType == (trig->trigArgs)[2].u || (trig->trigArgs)[2].u == 0xFFFFFFFF) && g->winTeam != redirect_player->teamSide) {
        result.i = 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_DrawGame:  // DrawGame
      if (g->winTeam == 0 && g->settledType != 0) {
        result.i = 1;
      } else {
        result.i = 0;
      }
      break;
    case ID_TRIG_MatchOver:  // MatchOver
      result.i = (GameMatchWinner() != 0);
      break;
    case ID_TRIG_GameTime:  // GameTime
      result.i = g->gameTime;
      break;
    case ID_TRIG_RoundNo:  // RoundNo
      result.i = g->roundNo;
      break;
    case ID_TRIG_RoundState:  // RoundState
      result.i = g->roundState;
      break;
    case ID_TRIG_MatchNo:  // MatchNo
      result.i = g->matchNo;
      break;
    case ID_TRIG_RoundsExisted:  // RoundsExisted
      result.i = redirect_player->roundsExisted;
      break;
    case ID_TRIG_TeamSide:  // TeamSide
      result.i = redirect_player->teamSide;
      break;
    case ID_TRIG_TeamMode: {  // TeamMode = $0
      int32_t temp = (&(g->p1CurrentTurn))[redirect_player->turnCharsetIdx].isSimul;
      if (*((&(g->p1TotalTurns)) + 6812 * redirect_player->turnCharsetIdx) > 1) {
        // If my team total turns > 1 then...
        temp = 3;
      }
      // temp = 0 -> Single
      //        1 -> Simul
      //        3 -> Turns
      result.i = (temp == (trig->trigArgs)[0].i);
    } break;
    case ID_TRIG_IsHomeTeam:  // IsHomeTeam
      if (g->isHomeTeamRight == 0) {
        result.i = (redirect_player->teamSide == 1);
      } else {
        result.i = (redirect_player->teamSide == 3 - g->isHomeTeamRight);
      }
      break;
    case ID_TRIG_NumExplod:  // NumExlod
      result.i = ExplodCount(g->explods, redirect_player->playerId, -1);
      break;
    case ID_TRIG_NumTarget:  // NumTarget
      result.i = TargetCount(redirect_player->targets, -1);
      break;
    case ID_TRIG_GetHitVar:  // GetHitVar($0)
      switch ((MUGEN_GET_HIT_VAR_ID)((trig->trigArgs)[0].i)) {
        case ID_GHV_XVelAdd:
          result.i = (redirect_player->getHitVars).XVelAdd;
          break;
        case ID_GHV_YVelAdd:
          result.i = (redirect_player->getHitVars).YVelAdd;
          break;
        case ID_GHV_Type:
          result.i = (redirect_player->getHitVars).type;
          break;
        case ID_GHV_AnimType:
          result.i = (redirect_player->getHitVars).animType;
          break;
        case ID_GHV_AirType:
          result.i = (redirect_player->getHitVars).airType;
          break;
        case ID_GHV_GroundType:
          result.i = (redirect_player->getHitVars).groundType;
          break;
        case ID_GHV_Damage:
          result.i = (redirect_player->getHitVars).damage;
          break;
        case ID_GHV_HitCount:
          result.i = (redirect_player->getHitVars).hitCount;
          break;
        case ID_GHV_FallCount:
          result.i = (redirect_player->getHitVars).fallCount;
          break;
        case ID_GHV_HitShakeTime:
          result.i = (redirect_player->getHitVars).hitShakeTime;
          break;
        case ID_GHV_HitTime:
          result.i = (redirect_player->getHitVars).hitTime;
          break;
        case ID_GHV_SlideTime:
          result.i = (redirect_player->getHitVars).slideTime;
          break;
        case ID_GHV_CtrlTime:
          result.i = (redirect_player->getHitVars).ctrlTime;
          break;
        case ID_GHV_RecoverTime:
          result.i = (redirect_player->getHitVars).recoverTime;
          break;
        case ID_GHV_XOff:
          result.f = (redirect_player->getHitVars).XOff;
          break;
        case ID_GHV_YOff:
          result.f = (redirect_player->getHitVars).YOff;
          break;
        case ID_GHV_ZOff:
          result.f = (redirect_player->getHitVars).ZOff;
          break;
        case ID_GHV_XVel:
          result.f = (redirect_player->getHitVars).XVel;
          break;
        case ID_GHV_YVel:
          result.f = (redirect_player->getHitVars).YVel;
          break;
        case ID_GHV_YAccel:
          result.f = (redirect_player->getHitVars).YAccel;
          break;
        case ID_GHV_ChainID:
          result.i = (redirect_player->getHitVars).chainId;
          break;
        case ID_GHV_Guarded:
          result.i = (redirect_player->getHitVars).guarded;
          break;
        case ID_GHV_IsBound:
          result.i = ((redirect_player->getHitVars).boundedBy != NULL);
          break;
        case ID_GHV_Fall:
          result.i = (redirect_player->getHitVars).fall;
          break;
        case ID_GHV_Fall_Damage:
          result.i = (redirect_player->getHitVars).fall_damage;
          break;
        case ID_GHV_Fall_XVel:
          result.f = (redirect_player->getHitVars).fall_XVel;
          break;
        case ID_GHV_Fall_YVel:
          result.f = (redirect_player->getHitVars).fall_YVel;
          break;
        case ID_GHV_Fall_Recover:
          result.i = (redirect_player->getHitVars).fall_recover;
          break;
        case ID_GHV_Fall_Time:
          result.i = (redirect_player->getHitVars).fall_time;
          break;
        case ID_GHV_Fall_RecoverTime:
          result.i = (redirect_player->getHitVars).fall_recoverTime;
          break;
        case ID_GHV_Fall_Kill:
          result.i = (redirect_player->getHitVars).fall_kill;
          break;
        case ID_GHV_Fall_EnvShake_Time:
          result.i = (redirect_player->getHitVars).fall_envShake.time;
          break;
        case ID_GHV_Fall_EnvShake_Dir:
          result.i = (redirect_player->getHitVars).fall_envShake.dir;
          break;
        case ID_GHV_Fall_EnvShake_Ampl:
          result.i = (int32_t)((redirect_player->getHitVars).fall_envShake.ampl);
          break;
        case ID_GHV_Fall_EnvShake_Freq:
          result.f = (redirect_player->getHitVars).fall_envShake.freq;
          break;
        case ID_GHV_Fall_EnvShake_Phase:
          result.f = (redirect_player->getHitVars).fall_envShake.phase;
          break;
        default:
          if (useRedirect == 0) {
            Warn(redirect_player, "Illegal gethitvar - programmer's fault");
            result.i = 0; /* 実際は、大抵は0になるが、0x47B168を通っていた場合0以外が入る可能性もある */
          } else {
            WarnR(p1, redirect_player, "Illegal gethitvar - programmer's fault");
            result.i = 0; /* 同上, 明らかにミスだが、隔離を使わない限りdefaultが実行されることは無いのでほぼ問題なし */
          }
      }
      break;
    case ID_TRIG_Const: {  // Const($0)
      int32_t temp = (trig->trigArgs)[0].i;
      if (CnstType[temp] == 0) {
        result.i = *(int32_t*)((int32_t)redirect_player + temp * 0x4 + 0x50);
      } else {
        result.f = *(float*)((int32_t)redirect_player + temp * 0x4 + 0x50);
      }
    } break;
    case ID_TRIG_TicksPerSecond:  // TicksPerSecond
      result.i = TimerFPS;
      break;
    case ID_TRIG_IsHelper:  // IsHelper
      result.i = ((redirect_player->isHelper) == 1);
      break;
    case ID_TRIG_NumHelper:  // NumHelper
      result.i = HelperCount(redirect_player, 0);
      break;
    case ID_TRIG_ParentDist:  // ParentDist X / ParentDist Y / ParentDist Z, $0で判別
      if (redirect_parent == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      if ((trig->trigArgs[0]).i == 0) {
        // ParentDist X
        result.f = (float)PlayerDistX(redirect_player, redirect_parent);
      } else if ((trig->trigArgs)[0].i == 1) {
        // ParentDist Y
        result.f = redirect_parent->posY - redirect_player->posY;
      } else if ((trig->trigArgs)[0].i == 2) {
        // ParentDist Z
        result.f = redirect_parent->posZ - redirect_player->posZ;
      }
      break;
    case ID_TRIG_RootDist:  // RootDist X / RootDist Y / RootDist Z, $0で判別
      if (redirect_root == NULL) {
        PushMyStackI(0);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      }
      if ((trig->trigArgs[0]).i == 0) {
        // RootDist X
        result.f = (float)PlayerDistX(redirect_player, redirect_root);
      } else if ((trig->trigArgs)[0].i == 1) {
        // RootDist Y
        result.f = redirect_root->posY - redirect_player->posY;
      } else if ((trig->trigArgs)[0].i == 2) {
        // RootDist Z
        result.f = redirect_root->posZ - redirect_player->posZ;
      }
      break;
    case ID_TRIG_StandBy:  // StandBy
      result.i = redirect_player->standBy;
      break;
    case ID_TRIG_Pi:  // Pi
      result.f = 3.1415927410125732F;
      break;
    case ID_TRIG_E:  // E
      result.f = 2.7182817459106445F;
      break;
    case ID_TRIG_IfElse: {  // IfElse(c, b, a)
      MUGEN_EVAL_TYPE a_type = (PopMyStack()).t;
      MUGEN_EVAL_NODE a = PopMyStack();
      MUGEN_EVAL_TYPE b_type = (PopMyStack()).t;
      MUGEN_EVAL_NODE b = PopMyStack();
      MUGEN_EVAL_TYPE c_type = (PopMyStack()).t;
      MUGEN_EVAL_NODE c = PopMyStack();
      if (c_type == EVAL_TYPE_TRIGGER) {
        PushMyStack(c);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else if (b_type == EVAL_TYPE_TRIGGER) {
        PushMyStack(b);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else if (a_type == EVAL_TYPE_TRIGGER) {
        PushMyStack(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        bool flag = true;
        // c_type == EVAL_TYPE_FLOATのときだけ特殊扱いしようとしてミスしたけど, 結果的に問題なかったみたいな記述
        if (c_type == EVAL_TYPE_OPERATOR) {
          if (c.f == 0.0F) {
            flag = false;
          }
        } else {
          if (c.i == 0) {
            flag = false;
          }
        }
        if (flag) {
          PushMyStack(b);
          PushMyStackT(b_type);
          return;
        } else {
          PushMyStack(a);
          PushMyStackT(a_type);
          return;
        }
      }
    } break;
    case ID_TRIG_Log: {  // Log(b, a)
      float a, b;
      uint32_t result1 = SPopF(player, &a);
      uint32_t result2 = SPopF(player, &b);
      if (result2 == 3) {
        PushMyStackI((int32_t)b);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else if (result1 == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a <= 0.0F || a == 1.0F || b <= 0.0F) {
          if (useRedirect == 0) {
            Warn(redirect_player, "took log with non-positive base or argument.");
          } else {
            WarnR(p1, redirect_player, "took log with non-positive base or argument.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.f = (__builtin_log2f(a) / __builtin_log2f(b));
        }
      }
    } break;
    case ID_TRIG_Sin: {  // Sin(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.f = __builtin_sinf(a);
      }
    } break;
    case ID_TRIG_Cos: {  // Cos(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.f = __builtin_cosf(a);
      }
    } break;
    case ID_TRIG_Tan: {  // Tan(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.f = __builtin_tanf(a);
      }
    } break;
    case ID_TRIG_ASin: {  // ASin(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < -1.0F || a > 1.0F) {
          if (useRedirect == 0) {
            Warn(redirect_player, "asin argument out of range");
          } else {
            WarnR(p1, redirect_player, "asin argument out of range");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.f = __builtin_asinf(a);
        }
      }
    } break;
    case ID_TRIG_ACos: {  // ACos(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < -1.0F || a > 1.0F) {
          if (useRedirect == 0) {
            Warn(redirect_player, "acos argument out of range");
          } else {
            WarnR(p1, redirect_player, "acos argument out of range");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.f = __builtin_acosf(a);
        }
      }
    } break;
    case ID_TRIG_ATan: {  // ATan(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.f = __builtin_atanf(a);
      }
    } break;
    case ID_TRIG_Exp: {  // Exp(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.f = __builtin_expf(a);
      }
    } break;
    case ID_TRIG_Ln: {  // Ln(a)
      float a;
      if (SPopF(player, &a) == 3) {
        PushMyStackI((int32_t)a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a <= 0.0F) {
          if (useRedirect == 0) {
            Warn(redirect_player, "took ln with non-positive number.");
          } else {
            WarnR(p1, redirect_player, "took ln with non-positive number.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.f = __builtin_logf(a);
        }
      }
    } break;
    case ID_TRIG_Abs: {  // Abs(a)
      MUGEN_EVAL_TYPE a_type = (PopMyStack()).t;
      MUGEN_EVAL_NODE a = PopMyStack();
      if (a_type == EVAL_TYPE_ERROR) {
        PushMyStackI(a.i);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else if (a_type == EVAL_TYPE_INT) {
        PushMyStackI(__builtin_abs(a.i));
        PushMyStackT(EVAL_TYPE_INT);
        return;
      } else if (a_type == EVAL_TYPE_FLOAT) {
        PushMyStackF(__builtin_fabsf(a.f));
        PushMyStackT(EVAL_TYPE_FLOAT);
        return;
      } else {
        sprintf((char*)ErrorMessage, "Panic: aborting");
        ErrorExit();
      }
    } break;
    case ID_TRIG_Floor:   // Floor(a)
    case ID_TRIG_Ceil: {  // Ceil(a)
      MUGEN_EVAL_TYPE a_type = (PopMyStack()).t;
      MUGEN_EVAL_NODE a = PopMyStack();
      if (a_type == EVAL_TYPE_ERROR) {
        PushMyStackI(a.i);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else if (a_type == EVAL_TYPE_INT) {
        result.i = a.i;
      } else if (a_type == EVAL_TYPE_FLOAT) {
        if (trig->trigID == ID_TRIG_Floor) {
          result.i = (int32_t)(__builtin_floor((double)a.f));
        } else {
          result.i = (int32_t)(__builtin_ceil((double)a.f));
        }
      } else {
        sprintf((char*)ErrorMessage, "Panic: aborting");
        ErrorExit();
      }
    } break;
    case ID_TRIG_AnimElemTime: {  // AnimElemTime(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        int32_t temp = AnimCurElemStartTime(redirect_player->anim, a - 1);
        if (temp == -1) {
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.i = redirect_player->anim->currentFrame - temp;
        }
      }
    } break;
    case ID_TRIG_AnimExist: {  // AnimExist(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.i = PlayerAnimExist(redirect_player, a);
      }
    } break;
    case ID_TRIG_SelfAnimExist: {  // SelfAnimExist(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.i = PlayerSelfAnimExist(redirect_player, a);
      }
    } break;
    case ID_TRIG_PlayerIDExist: {  // PlayerIDExist(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.i = (PlayerAccessID(a) != NULL);
      }
    } break;
    case ID_TRIG_IsHelper_ID: {  // IsHelper(a)
      int32_t a;
      uint32_t ret = SPopI(redirect_player, &a);
      if (redirect_player->isHelper != 1) {
        result.i = 0;
      } else {
        if (ret == 3) {
          PushMyStackI(a);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.i = (redirect_player->playerId == a);
        }
      }
    } break;
    case ID_TRIG_NumHelper_ID: {  // NumHelper(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        result.i = HelperCount(redirect_player, a);
      }
    } break;
    case ID_TRIG_ProjHitTime: {  // ProjHitTime(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0) {
          a = 0;
          if (useRedirect == 0) {
            Warn(redirect_player, "Negative projectile ID specified.");
          } else {
            WarnR(p1, redirect_player, "Negative projectile ID specified.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        if (redirect_player->projContactType == PROJCONTACT_HIT && (redirect_player->projContactId == a || a == 0)) {
          result.i = redirect_player->projContactTime;
        } else {
          result.i = -1;
        }
      }
    } break;
    case ID_TRIG_ProjGuardedTime: {  // ProjGuardedTime(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0) {
          a = 0;
          if (useRedirect == 0) {
            Warn(redirect_player, "Negative projectile ID specified.");
          } else {
            WarnR(p1, redirect_player, "Negative projectile ID specified.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        if (redirect_player->projContactType == PROJCONTACT_GUARDED && (redirect_player->projContactId == a || a == 0)) {
          result.i = redirect_player->projContactTime;
        } else {
          result.i = -1;
        }
      }
    } break;
    case ID_TRIG_ProjContactTime: {
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0) {
          a = 0;
          if (useRedirect == 0) {
            Warn(redirect_player, "Negative projectile ID specified.");
          } else {
            WarnR(p1, redirect_player, "Negative projectile ID specified.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        if (redirect_player->projContactType != PROJCONTACT_NONE && redirect_player->projContactType != PROJCONTACT_CANCEL && (redirect_player->projContactId == a || a == 0)) {
          result.i = redirect_player->projContactTime;
        } else {
          result.i = -1;
        }
      }
    } break;
    case ID_TRIG_NumProjID: {  // NumProjID(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0) {
          a = 0;
          if (useRedirect == 0) {
            Warn(redirect_player, "Negative projectile ID specified.");
          } else {
            WarnR(p1, redirect_player, "Negative projectile ID specified.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        result.i = ProjArrayCountActiveID(redirect_player->projectiles, a);
      }
    } break;
    case ID_TRIG_NumExplod_ID: {  // NumExplod(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < -1) {
          a = -1;
          if (useRedirect == 0) {
            Warn(redirect_player, "illegal explod ID for numexplod");
          } else {
            WarnR(p1, redirect_player, "illegal explod ID for numexplod");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        result.i = ExplodCount(g->explods, redirect_player->playerId, a);
      }
    } break;
    case ID_TRIG_NumTarget_ID: {  // NumTarget(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < -1) {
          a = -1;
          if (useRedirect == 0) {
            Warn(redirect_player, "illegal target ID for numtarget");
          } else {
            WarnR(p1, redirect_player, "illegal target ID for numtarget");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        result.i = TargetCount(redirect_player->targets, a);
      }
    } break;
    case ID_TRIG_Var: {  // Var(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0 || a > 59) {
          if (useRedirect == 0) {
            Warn(redirect_player, "var index out of range");
          } else {
            WarnR(p1, redirect_player, "var index out of range");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.i = *(int32_t*)((int32_t)(&(redirect_player->var)) + a * 0x4);
        }
      }
    } break;
    case ID_TRIG_FVar: {  // FVar(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0 || a > 39) {
          if (useRedirect == 0) {
            Warn(redirect_player, "fvar index out of range");
          } else {
            WarnR(p1, redirect_player, "fvar index out of range");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.f = *(float*)((int32_t)(&(redirect_player->var)) + a * 0x4);
        }
      }
    } break;
    case ID_TRIG_SysVar: {  // SySvar(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0 || a > 4) {
          if (useRedirect == 0) {
            Warn(redirect_player, "sysvar index out of range");
          } else {
            WarnR(p1, redirect_player, "sysvar index out of range");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.i = *(int32_t*)((int32_t)(&(redirect_player->sysvar)) + a * 0x4);
        }
      }
    } break;
    case ID_TRIG_SysFVar: {  // SysFVar(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0 || a > 4) {
          if (useRedirect == 0) {
            Warn(redirect_player, "sysfvar index out of range");
          } else {
            WarnR(p1, redirect_player, "sysfvar index out of range");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        } else {
          result.i = *(float*)((int32_t)(&(redirect_player->sysfvar)) + a * 0x4);
        }
      }
    } break;
    case ID_TRIG_ProjCancelTime: {  // ProjCancelTime(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        if (a < 0) {
          a = 0;
          if (useRedirect == 0) {
            Warn(redirect_player, "Negative projectile ID specified.");
          } else {
            WarnR(p1, redirect_player, "Negative projectile ID specified.");
          }
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
        if (redirect_player->projContactType == PROJCONTACT_CANCEL && (redirect_player->projContactId == a || a == 0)) {
          result.i = redirect_player->projContactTime;
        } else {
          result.i = -1;
        }
      }
    } break;
    case ID_TRIG_AnimElemNo: {  // AnimElemNo(a)
      int32_t a;
      if (SPopI(redirect_player, &a) == 3) {
        PushMyStackI(a);
        PushMyStackT(EVAL_TYPE_ERROR);
        return;
      } else {
        int32_t temp = AnimElemNo(redirect_player->anim, a);
        if (temp != -1) {
          result.i = temp;
        } else {
          PushMyStackI(0);
          PushMyStackT(EVAL_TYPE_ERROR);
          return;
        }
      }
    } break;
    default:
      sprintf((char*)(g->errorMessage), "Missing trigger code %i", trig->trigID);
      ErrorExit();
      break;
  }

  if (HasCompTrig(trig->trigID)) {
    switch (trig->compareStyle) {
      case EVAL_EQ:
        result.i = ((result.i) == ((trig->trigArgs)[0].i));
        break;
      case EVAL_NEQ:
        result.i = ((result.i) != ((trig->trigArgs)[0].i));
        break;
      case EVAL_GT:
        result.i = ((result.i) > ((trig->trigArgs)[0].i));
        break;
      case EVAL_GTE:
        result.i = ((result.i) >= ((trig->trigArgs)[0].i));
        break;
      case EVAL_LT:
        result.i = ((result.i) < ((trig->trigArgs)[0].i));
        break;
      case EVAL_LTE:
        result.i = ((result.i) <= ((trig->trigArgs)[0].i));
        break;
      case EVAL_RANGE_IN_IN:
        if ((result.i) < ((trig->trigArgs)[0].i) || ((trig->trigArgs)[1].i) < result.i) {
          result.i = 0;
        } else {
          result.i = 1;
        }
        break;
      case EVAL_RANGE_IN_OUT:
        if ((result.i) < ((trig->trigArgs)[0].i) || ((trig->trigArgs)[1].i) <= result.i) {
          result.i = 0;
        } else {
          result.i = 1;
        }
        break;
      case EVAL_RANGE_OUT_IN:
        if ((result.i) <= ((trig->trigArgs)[0].i) || ((trig->trigArgs)[1].i) < result.i) {
          result.i = 0;
        } else {
          result.i = 1;
        }
        break;
      case EVAL_RANGE_OUT_OUT:
        if ((result.i) <= ((trig->trigArgs)[0].i) || ((trig->trigArgs)[1].i) <= result.i) {
          result.i = 0;
        } else {
          result.i = 1;
        }
        break;
    }
    if (result_inversed != 0) {
      result.i = !(result.i);
    }
  }

  if (trig->isFloat == 0) {
    PushMyStack(result);
    PushMyStackT(EVAL_TYPE_INT);
  } else if (trig->isFloat == 1) {
    PushMyStack(result);
    PushMyStackT(EVAL_TYPE_FLOAT);
  } else {
    ErrorExit();
  }
}
