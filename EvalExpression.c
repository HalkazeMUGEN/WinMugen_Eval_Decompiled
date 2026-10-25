/**
 * @file EvalExpression.c
 * @author Halkaze
 * @brief EvalExpression（式評価処理）関連関数群
 * @date 2023-03-17
 * @copyright Copyright (c) 2023
 *
 * 留意事項
 * ・コンパイル結果がwinmugenのものとまったく同一となることは保証できません
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

int32_t R_EvalExpressionI(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval, uint32_t noWarning) __attribute__((nonnull(1, 2)));
float R_EvalExpressionF(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval) __attribute__((nonnull(1, 2)));
int32_t R_EvalExpression(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval, int32_t* pInt, float* pFloat) __attribute__((nonnull(1, 2, 3, 4)));

int32_t R_EvalExpressionI(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval, uint32_t noWarning) {
  int32_t intResult;
  float floatResult;
  int32_t result = EvalExpression(player, eval, &intResult, &floatResult);
  if (result == 0) {
    sprintf((char*)ErrorMessage, "%4i: Fatal: Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
    ErrorExit();
  } else if (result == 2) {
    intResult = (int32_t)(floatResult);
    if (noWarning != 0) {
      ClipboardAppend(g->warningClipboard, "%i Warning: expression truncated to integer:\nplayer %s (%i) in state %i: %f", g->gameTime, player->debugName, player->playerId, player->stateNo, floatResult);
    }
  } else if (result == 3) {
    intResult = 0;
    ClipboardAppend(g->warningClipboard, "%i Warning: expression short-circuited, returning 0:\nplayer %s (%i) in state %i", g->gameTime, player->debugName, player->playerId, player->stateNo);
  }
  return intResult;
}

float R_EvalExpressionF(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval) {
  int32_t intResult;
  float floatResult;
  int32_t result = EvalExpression(player, eval, &intResult, &floatResult);
  if (result == 0) {
    sprintf((char*)ErrorMessage, "%4i: Fatal: Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
    ErrorExit();
  } else if (result == 1) {
    floatResult = (float)(intResult);
  } else if (result == 3) {
    floatResult = 0.0F;
    ClipboardAppend(g->warningClipboard, "%i Warning: expression short-circuited, returning 0:\nplayer %s (%i) in state %i", g->gameTime, player->debugName, player->playerId, player->stateNo);
  }
  return floatResult;
}

#define PushMyStackI(data) PushMyStack((int32_t)(data))
#define PushMyStackF(data) PushMyStack((float)(data))
#define PushMyStackT(data) PushMyStack((MUGEN_EVAL_TYPE)(data))
#define PushMyStack(data) PushMyStack_((MUGEN_EVAL_NODE)(data))
static inline void PushMyStack_(MUGEN_EVAL_NODE data);
static inline MUGEN_EVAL_NODE PopMyStack(MUGEN_PLAYER* player) __attribute__((nonnull(1)));

static inline void PushMyStack_(MUGEN_EVAL_NODE data) {
  if (MyStackIndex > 126) {
    sprintf((char*)ErrorMessage, "Expression stack overflow.");
    ErrorExit();
  } else {
    MyStack[++MyStackIndex] = data;
  }
}

static inline MUGEN_EVAL_NODE PopMyStack(MUGEN_PLAYER* player) {
  if (MyStackIndex < 0) {
    sprintf((char*)ErrorMessage, "%4i: Fatal: Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
    ErrorExit();
  }
  return MyStack[MyStackIndex--];
}

#define TYPE_ERROR_OP() TYPE_ERROR_(player, (eval->exprs)[idx].op, pIdx)
#define TYPE_ERROR() TYPE_ERROR_(player, (eval->exprs)[idx].op, &idx)
static inline void TYPE_ERROR_(MUGEN_PLAYER* player, MUGEN_EVAL_OPERATOR op, int32_t* pIdx) __attribute__((nonnull(1, 3)));
static inline void TYPE_ERROR_(MUGEN_PLAYER* player, MUGEN_EVAL_OPERATOR op, int32_t* pIdx) {
  switch (op) {
    case EVAL_OP_RANGE_OUT_OUT:
    case EVAL_OP_RANGE_IN_OUT:
    case EVAL_OP_RANGE_OUT_IN:
    case EVAL_OP_RANGE_IN_IN:
      ++(*pIdx);
      break;
    case EVAL_OP_POW:
      Warn(player, "Invalid exponentiation");
      break;
    case EVAL_OP_IASSIGN:
    case EVAL_OP_FASSIGN:
      Warn(player, "Short-circuited var assign ignored");
      break;
    case EVAL_OP_DIV:
      Warn(player, "Division by zero");
      break;
    case EVAL_OP_MOD:
      Warn(player, "Modulus by zero");
      break;
    default:
      break;
  }
  PushMyStackI(0);
  PushMyStackT(EVAL_TYPE_ERROR);
}

static inline void EvalOperator(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval, int32_t idx, int32_t* pIdx) __attribute__((nonnull(1, 2, 4)));

int32_t R_EvalExpression(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval, int32_t* pInt, float* pFloat) {
  if (eval->types == NULL) {
    if ((int32_t)(eval->exprs) == 0) {
      *pInt = (eval->value).i;
      return 1;
    } else if ((int32_t)(eval->exprs) == 1) {
      *pFloat = (eval->value).f;
      return 2;
    } else {
      return 0;
    }
  }

  int32_t preMyStackIndex = MyStackIndex;
  for (int32_t idx = 0; idx < (eval->value).i; ++idx) {
    switch (eval->types[idx]) {
      case EVAL_TYPE_INT:
        PushMyStackI((eval->exprs)[idx].i);
        PushMyStackT((eval->types)[idx]);
        break;
      case EVAL_TYPE_FLOAT:
        PushMyStackF((eval->exprs)[idx].f);
        PushMyStackT((eval->types)[idx]);
        break;
      case EVAL_TYPE_OPERATOR:
        EvalOperator(player, eval, idx, &idx);
        break;
      case EVAL_TYPE_TRIGGER:
        SCtrlTrigEval(player, (eval->exprs)[idx].trig);
        break;
      default:
        TYPE_ERROR();
        break;
    }
  }

  if (MyStackIndex - preMyStackIndex == 2) {
    MUGEN_EVAL_TYPE returnType = (PopMyStack(player)).t;
    if (returnType == EVAL_TYPE_INT || returnType == EVAL_TYPE_ERROR) {
      *pInt = (PopMyStack(player)).i;
      return 1;
    } else if (returnType == EVAL_TYPE_FLOAT) {
      *pFloat = (PopMyStack(player)).f;
      return 2;
    } else {
      return 0;
    }
  }

  sprintf((char*)ErrorMessage, "%4i: Fatal: Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
  ErrorExit();
  return 0;
}

static inline void EvalOperator(MUGEN_PLAYER* player, MUGEN_EVAL_VALUE* eval, int32_t idx, int32_t* pIdx) {
  MUGEN_EVAL_TYPE a_type, b_type;
  MUGEN_EVAL_NODE a, b;

  a_type = (PopMyStack(player)).t;
  a = PopMyStack(player);
  uint32_t type = 1;
  if (a_type == EVAL_TYPE_FLOAT) {
    type = 2;
  }

  if ((eval->exprs)[idx].op != EVAL_OP_LNOT && (eval->exprs)[idx].op != 0x7E && (eval->exprs)[idx].op != EVAL_OP_NEG) {
    b_type = (PopMyStack(player)).t;
    if (b_type == EVAL_TYPE_FLOAT) {
      if (type == 1) {
        a.f = (float)(a.i);
      }
      type = 2;
    }
    b = PopMyStack(player);
    if (type == 2 && b_type == EVAL_TYPE_INT) {
      b.f = (float)(b.i);
    }
  }

  switch ((eval->exprs)[idx].op) {
    case EVAL_OP_EQ:
    case EVAL_OP_NEQ:
    case EVAL_OP_GT:
    case EVAL_OP_LT:
    case EVAL_OP_GTE:
    case EVAL_OP_LTE:
    case EVAL_OP_RANGE_OUT_OUT:
    case EVAL_OP_RANGE_IN_OUT:
    case EVAL_OP_RANGE_OUT_IN:
    case EVAL_OP_RANGE_IN_IN:
    case EVAL_OP_LAND:
    case EVAL_OP_LOR:
    case EVAL_OP_LNOT:
    case EVAL_OP_LXOR:
    case EVAL_OP_POW:
    case EVAL_OP_NEG:
    case EVAL_OP_FASSIGN:
    case EVAL_OP_ADD:
    case EVAL_OP_SUB:
    case EVAL_OP_MUL:
    case EVAL_OP_DIV:
    case EVAL_OP_UNKNOWN1:
    case EVAL_OP_UNKNOWN2:
      break;
    case EVAL_OP_BAND:
    case EVAL_OP_BOR:
    case EVAL_OP_BXOR:
    case EVAL_OP_IASSIGN:
    case EVAL_OP_BNOT:
    case EVAL_OP_MOD:
      if (type == 1) {
        break;
      }
      /* fall through */
    default:
      sprintf((char*)(g->errorMessage), "Type mismatch detected: %d", (eval->exprs)[idx].op);
      ShowExp((char*)ErrorMessage, eval, player);
      ErrorExit();
  }

  switch ((eval->exprs)[idx].op) {
    case EVAL_OP_EQ:
    case EVAL_OP_NEQ: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = (b.i == a.i);
      } else {
        result.i = (b.f == a.f);
      }
      if ((eval->exprs)[idx].op != EVAL_OP_EQ) {
        result.i = !(result.i);
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_GT:
    case EVAL_OP_LTE: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = (b.i > a.i);
      } else {
        result.i = (b.f > a.f);
      }
      if ((eval->exprs)[idx].op != EVAL_OP_GT) {
        result.i = !(result.i);
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_LT:
    case EVAL_OP_GTE: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = (b.i < a.i);
      } else {
        result.i = (b.f < a.f);
      }
      if ((eval->exprs)[idx].op != EVAL_OP_LT) {
        result.i = !(result.i);
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_RANGE_OUT_OUT:
    case EVAL_OP_RANGE_IN_OUT:
    case EVAL_OP_RANGE_OUT_IN:
    case EVAL_OP_RANGE_IN_IN: {
      MUGEN_EVAL_NODE result;
      if (eval->types[idx + 1] == EVAL_TYPE_OPERATOR && (((eval->exprs)[idx + 1].op) == EVAL_OP_EQ || ((eval->exprs)[idx + 1].op) == EVAL_OP_NEQ)) {
        MUGEN_EVAL_TYPE x_type = (PopMyStack(player)).t;
        MUGEN_EVAL_NODE x = PopMyStack(player);
        if (x_type == EVAL_TYPE_ERROR) {
          TYPE_ERROR_OP();
        } else {
          result.i = ((eval->exprs)[idx + 1].op == EVAL_OP_EQ);
          if (type == 1 && x_type == EVAL_TYPE_FLOAT) {
            a.f = (float)(a.i);
            b.f = (float)(b.i);
            type = 2;
          } else if (type == 2 && x_type == EVAL_TYPE_INT) {
            x.f = (float)(x.i);
          }
          if (type == 1) {
            switch ((eval->exprs)[idx].op) {
              case EVAL_OP_RANGE_OUT_OUT:
                if (x.i <= b.i || a.i <= x.i) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              case EVAL_OP_RANGE_IN_OUT:
                if (x.i <= b.i || a.i < x.i) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              case EVAL_OP_RANGE_OUT_IN:
                if (x.i < b.i || a.i <= x.i) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              case EVAL_OP_RANGE_IN_IN:
                if (x.i < b.i || a.i < x.i) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              default:
                TYPE_ERROR_OP();
                break;
            }
          } else {
            switch ((eval->exprs)[idx].op) {
              case EVAL_OP_RANGE_OUT_OUT:
                if (x.f <= b.f || a.f <= x.f) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              case EVAL_OP_RANGE_IN_OUT:
                if (x.f <= b.f || a.f < x.f) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              case EVAL_OP_RANGE_OUT_IN:
                if (x.f < b.f || a.f <= x.f) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              case EVAL_OP_RANGE_IN_IN:
                if (x.f < b.f || a.f < x.f) {
                  result.i = !(result.i);
                }
                PushMyStack(result);
                PushMyStackT(EVAL_TYPE_INT);
                ++(*pIdx);
                ++idx;
                break;
              default:
                TYPE_ERROR_OP();
                break;
            }
          }
        }
      } else {
        sprintf((char*)ErrorMessage, "%4i: Fatal: Player %s (%i) in state %i: malformed expression", g->gameTime, player->debugName, player->playerId, player->stateNo);
        ErrorExit();
      }
    } break;
    case EVAL_OP_LAND: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = (a.i && b.i);
      } else {
        result.i = (a.f && b.f);
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_LOR: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = (a.i || b.i);
      } else {
        result.i = (a.f || b.f);
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_LNOT: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = (a.i == 0);
      } else {
        result.i = (a.f == 0.0F);
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_LXOR: {
      MUGEN_EVAL_NODE result;
      if (type == 1) {
        result.i = ((a.i == 0 && b.i != 0) || (a.i != 0 && b.i == 0));
      } else {
        result.i = ((a.f == 0 && b.f != 0) || (a.f != 0 && b.f == 0));
      }
      PushMyStack(result);
      PushMyStackT(EVAL_TYPE_INT);
    } break;
    case EVAL_OP_POW:
      if (type == 1 && a.i <= -1) {
        b.f = (float)(b.i);
        a.f = (float)(a.i);
        type = 2;
      }
      if (type == 1) {
        MUGEN_EVAL_NODE result;
        if (a.i == 0) {
          result.i = 1;
        } else {
          int32_t temp = 1;
          result.i = b.i;
          if (a.i > 1) {
            do {
              result.i = result.i * result.i;
              if ((a.i & (1 << ((temp - 1) & 0x1F))) != 0) {
                result.i = result.i * b.i;
              }
              temp++;
            } while ((1 << (temp & 0x1F)) <= a.i);
          }
        }
        PushMyStack(result);
        PushMyStackT(EVAL_TYPE_INT);
      } else {
        double temp = __builtin_pow((double)(b.f), (double)(a.f));
        if (__builtin_isnan(temp)) {
          TYPE_ERROR_OP();
        } else {
          PushMyStackF(temp);
          PushMyStackT(EVAL_TYPE_FLOAT);
        }
      }
      break;
    case EVAL_OP_BAND:
      PushMyStackI(b.i & a.i);
      PushMyStackT(EVAL_TYPE_INT);
      break;
    case EVAL_OP_BOR:
      PushMyStackI(b.i | a.i);
      PushMyStackT(EVAL_TYPE_INT);
      break;
    case EVAL_OP_BXOR:
      PushMyStackI(b.i ^ a.i);
      PushMyStackT(EVAL_TYPE_INT);
      break;
    case EVAL_OP_NEG:
      if (type == 1) {
        PushMyStackI(-(a.i));
        PushMyStackT(EVAL_TYPE_INT);
      } else {
        PushMyStackF(-(a.f));
        PushMyStackT(EVAL_TYPE_FLOAT);
      }
      break;
    case EVAL_OP_IASSIGN:
      if (type != 1) {
        a.i = (int32_t)(a.f);
        b.i = (int32_t)(b.f);
        Warn(player, "Float truncated to int in var assignment");
      }
      PushMyStackI(a.i);
      PushMyStackT(EVAL_TYPE_INT);
      if (b.i < 0 || b.i > 60) {
        Warn(player, "Assignment to illegal variable ignored");
      } else {
        (player->var)[b.i] = a.i;
      }
      break;
    case EVAL_OP_FASSIGN:
      if (type == 1) {
        a.f = (float)(a.i);
      } else {
        b.i = (int32_t)(b.f);
      }
      PushMyStackF(a.f);
      PushMyStackT(EVAL_TYPE_FLOAT);
      if (b.i < 0 || b.i > 40) {
        Warn(player, "Assignment to illegal variable ignored");
      } else {
        (player->fvar)[b.i] = a.f;
      }
      break;
    case EVAL_OP_BNOT:
      PushMyStackI(~(a.i));
      PushMyStackT(EVAL_TYPE_INT);
      break;
    case EVAL_OP_ADD:
      if (type == 1) {
        PushMyStackI(b.i + a.i);
        PushMyStackT(EVAL_TYPE_INT);
      } else {
        PushMyStackF(b.f + a.f);
        PushMyStackT(EVAL_TYPE_FLOAT);
      }
      break;
    case EVAL_OP_SUB:
      if (type == 1) {
        PushMyStackI(b.i - a.i);
        PushMyStackT(EVAL_TYPE_INT);
      } else {
        PushMyStackF(b.f - a.f);
        PushMyStackT(EVAL_TYPE_FLOAT);
      }
      break;
    case EVAL_OP_MUL:
      if (type == 1) {
        PushMyStackI(b.i * a.i);
        PushMyStackT(EVAL_TYPE_INT);
      } else {
        PushMyStackF(b.f * a.f);
        PushMyStackT(EVAL_TYPE_FLOAT);
      }
      break;
    case EVAL_OP_DIV:
      if (type == 1) {
        if (a.i == 0) {
          TYPE_ERROR_OP();
        } else {
          PushMyStackI(b.i / a.i);
          PushMyStackT(EVAL_TYPE_INT);
        }
      } else {
        if (a.f == 0.0F) {
          TYPE_ERROR_OP();
        } else {
          PushMyStackF(b.f / a.f);
          PushMyStackT(EVAL_TYPE_FLOAT);
        }
      }
      break;
    case EVAL_OP_MOD:
      if (a.i == 0) {
        TYPE_ERROR_OP();
      } else {
        PushMyStackI(b.i % a.i);
        PushMyStackT(EVAL_TYPE_INT);
      }
      break;
    default:
      break;
  }
}
