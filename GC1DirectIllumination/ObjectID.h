#pragma once

enum class TObjectID : size_t { };

//=================================================================================
static const TObjectID c_invalidObjectID = (TObjectID)-1;
TObjectID g_lastObjectID = c_invalidObjectID;

//=================================================================================
TObjectID GenerateObjectID ()
{
    g_lastObjectID = (TObjectID)((size_t)(g_lastObjectID)+1);
    return g_lastObjectID;
}