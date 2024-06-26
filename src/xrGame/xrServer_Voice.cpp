#include "StdAfx.h"
#include "xrServer.h"

// VOICE
void xrServer::OnVoiceMessage(NET_Packet& P, ClientID sender)
{
	xrClientData* pClient = (xrClientData*)ID_to_client(sender);

	if (!pClient || !pClient->net_Ready) return;
	game_PlayerState* ps = pClient->ps;
	if (!ps) return;
	if (!pClient->owner) return;

	// Msg("VoiceMessage size: %u", P.B.count);

	struct send_voice_message
	{
		xrServer* m_server;
		NET_Packet* m_packet;
		xrClientData* m_from;
		float m_voiceDistanceSqr;

		void operator()(IClient* client)
		{
			if (client == m_server->GetServerClient())
				return;

			xrClientData* CL = static_cast<xrClientData*>(client);
			if (!CL || !CL->net_Ready || !CL->owner || !m_from->owner || !m_from->ps)
				return;

			if (CL->ID == m_from->ID)
				return;

			game_PlayerState* ps = CL->ps;
			if (!ps || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
				return;

			float distanceSqr = CL->owner->Position().distance_to_sqr(m_from->owner->Position());

			if (distanceSqr <= m_voiceDistanceSqr)
			{
				m_server->SendTo(CL->ID, *m_packet, net_flags(FALSE, TRUE, TRUE, TRUE));
			}
		}
	};

	u8 distance = P.r_u8(); // distance byte
	float voiceDistanceSqr = (float)distance * (float)distance;

	send_voice_message tmp_functor;
	tmp_functor.m_server = this;
	tmp_functor.m_packet = &P;
	tmp_functor.m_from = pClient;
	tmp_functor.m_voiceDistanceSqr = voiceDistanceSqr;

	ForEachClientDo(tmp_functor);
}