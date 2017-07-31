/*
* Copyright (C) 2017 Shaiya Genesis <http://www.shaiyagenesis.com/>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "PacketRepository.h"

#include <genesis/common/networking/packets/PacketBuilder.h>
#include <genesis/common/packets/Opcodes.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace Genesis::Game::Io::Packets::Outgoing;

/**
 * Sends the standard character data
 *
 * @param character
 *      The character instance to send the data to
 */
void PacketRepository::send_character_data(Genesis::Game::Model::Entity::Player::Character* character) {

	// Stats and OJs
    Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::CHARACTER_DETAILS); // Corrected
    
    // The character's attributes
    auto attributes = character->get_attributes();

    // Write the character's attributes
    bldr.write_short(attributes->get_strength()); // STR
    bldr.write_short(attributes->get_dexterity()); // DEX
    bldr.write_short(attributes->get_resistance()); // REC
    bldr.write_short(attributes->get_intelligence()); // INT
    bldr.write_short(attributes->get_wisdom()); // WIS
    bldr.write_short(attributes->get_luck()); // LUC
    bldr.write_short(character->get_stat_points()); // Stat points
    bldr.write_short(character->get_skill_points()); // Skill Points
    bldr.write_int(9); // Max HP
    bldr.write_int(10); // Max MP
    bldr.write_int(11); // Max SP

	// The direction of the character
	bldr.write_short(character->get_position()->get_direction());

	// EXP Values are multiplied by 10

	// Previous EXP was 1000
	bldr.write_int(100);

	// Next EXP is at 2500
	// Client takes the previous value, calculates the difference = 1500
	bldr.write_int(250); // 2500

	// Current EXP is at 1200
	// Client takes the previous value, difference = 200
	bldr.write_int(120); // 1200
 
 	// Client should display 200 / 1500 for EXP

    bldr.write_int(character->get_gold()); // Gold
 
    // The character's position
    auto position = character->get_position();
 
    // The position values
    float char_position_x = position->get_x();
    float char_position_height = position->get_height();
    float char_position_y = position->get_y();
 
    // The byte arrays of the character's position
    auto position_x = reinterpret_cast<unsigned char*>(&char_position_x);
    auto position_height = reinterpret_cast<unsigned char*>(&char_position_height);
    auto position_y = reinterpret_cast<unsigned char*>(&char_position_y);
 
    // Write the position values
    bldr.write_bytes(position_x, 4);
    bldr.write_bytes(position_height, 4);
    bldr.write_bytes(position_y, 4);
 
    bldr.write_int(character->get_kills()); // Kills
    bldr.write_int(character->get_deaths()); // Deaths
    bldr.write_int(character->get_victories()); // Victories
    bldr.write_int(character->get_defeats()); // Defeats
 
    bldr.write_byte(1); // Has guild
 
 	// The guild name
    std::string guild_name("Elitepvpers");

    // Write the guild name
    bldr.write_bytes((unsigned char*) guild_name.c_str(), guild_name.size());
 
    // Write the packet
    character->write(bldr.to_packet());
}

/**
 * Sends a notice to a character
 *
 * @param character
 *      The character to send the notice to
 *
 * @param message
 *      The un-formatted message
 *
 * @param args
 *      The arguments, to format into the message
 */
void PacketRepository::send_notice(Genesis::Game::Model::Entity::Player::Character* character, const char* message, std::initializer_list<const char*> args) {

	// The message
	auto boost_msg = boost::format(message);

	// Loop through the arguments
	for (auto arg : args) {

		// Define the message
		boost_msg = boost_msg % arg;

	}

	// The built message instance
	auto built_msg = boost::str(boost_msg);

	// Notice packet
    Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::NOTICE);

    // Write the length of the message
    bldr.write_byte(built_msg.size());

    // Write the message
    bldr.write_bytes((unsigned char*) built_msg.c_str(), built_msg.size());

    // Write the packet
    character->write(bldr.to_packet());

}

/**
 * Sends the current HP, MP and SP of the character
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_current_hp_mp_sp(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::CURRENT_CHARACTER_HITPOINTS);
    bldr.write_int(character->get_attributes()->get_current_hp()); // HP
    bldr.write_int(character->get_attributes()->get_current_mp()); // MP
    bldr.write_int(character->get_attributes()->get_current_sp()); // SP
    character->write(bldr.to_packet());
}

/**
 * Sends the player's current Aeria Points
 *
 * @param player
 *      The player instance
 */
void PacketRepository::send_ap(Genesis::Game::Model::Entity::Player::Player* player) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::ACCOUNT_AERIA_POINTS);
    bldr.write_int(player->get_points());
    player->write(bldr.to_packet());
}

/**
 * Sends the world's bless amount
 *
 * @param character
 *      The character to send the amount to
 */
void PacketRepository::send_bless_amount(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::BLESS_AMOUNT);
    unsigned char payload15[] = {0x00,
        0xC4,0x0E,
        0xFF,0x00,
        0xFF,0x00,
        0xFF,0x00};
    bldr.write_bytes(payload15, sizeof(payload15));
    character->write(bldr.to_packet());
}

/**
 * Sends the attack and movement speed of a character, to the character
 *
 * @param character
 *      The character to send the packet to
 *
 * @param target
 *      The target to send the speeds of
 */
void PacketRepository::send_attack_movement_speed(Genesis::Game::Model::Entity::Player::Character* character, Genesis::Game::Model::Entity::Player::Character* target) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::CHARACTER_ATTACK_MOVEMENT_SPEED);
    bldr.write_int(target->get_index());
    bldr.write_byte(9); // Atk target->get_attack_speed()
    bldr.write_byte(4); // Move target->get_movement_speed()
    character->write(bldr.to_packet());
}

/**
 * Sends the quest list for the character
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_quest_list(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::QUEST_LIST);
    unsigned char payload12[] = {
    	0x02, // Quest Count

    	0x49,0x0D, // Quest ID
    	0x00,0x00, // Remaining Time
    	0x00,0x00,0x00, // ?

    	0x4C,0x0D, // Quest ID
    	0x00,0x00, // Remaining Time
    	0x00,0x00,0x00}; // ?
    bldr.write_bytes(payload12, sizeof(payload12));
    character->write(bldr.to_packet());
}

/**
 * Sends the active skills/buffs for the character
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_active_buffs(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::CHARACTER_ACTIVE_BUFFS);
    unsigned char payload11[] = {0x01,
    	
    	0x94,0x1B,
    	0x00,0x00,
    	0xCA,0x00,0x01,0xB2,0x1B,0x00,0x00};
    bldr.write_bytes(payload11, sizeof(payload11));
    character->write(bldr.to_packet());
}

/**
 * Sends the skills learned by the character
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_learned_skills(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::CHARACTER_SKILLS);
    bldr.write_short(character->get_skill_points()); // Skill points
    bldr.write_byte(8); // Bytes per skill ?
 
 	// Strength training
 	bldr.write_short(610); // Skill ID
 	bldr.write_byte(1); // Skill Level
 	bldr.write_byte(0); // Skill Index
 	bldr.write_int(0); // Cooldown

    character->write(bldr.to_packet());
}

/**
 * Sends the guild list for the character's faction
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_faction_guild_list(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::GUILD_LIST);
    unsigned char payload7[] = {0x06,0x26,0x47,0x00,0x00,0x2D,0x52,0x65,0x64,0x5F,0x46,0x6C,0x61,0x6D,0x65,0x00,0x4F,0x66,0x20,0x57,0x72,0x61,0x74,0x68,0x00,0x65,0x61,0x74,0x68,0x00,0x41,0x70,0x70,0x70,0x72,0x6F,0x76,0x65,0x2E,0x2E,0x00,0x61,0x6C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x46,0x45,0x59,0x5A,0x00,0x61,0x61,0x61,0x61,0x61,0x61,0x61,0x00,0x65,0x20,0x50,0x4C,0x20,0x69,0x73,0x6C,0x61,0x6E,0x64,0x20,0x41,0x76,0x61,0x6C,0x6F,0x6E,0x20,0x3B,0x29,0x29,0x00,0xFB,0x20,0x47,0x72,0x6F,0x6D,0x5F,0x30,0x32,0x34,0x00,0x44,0x00,0x79,0x6F,0x75,0x20,0x64,0x6F,0x2E,0x00,0x67,0x75,0x69,0x6C,0x64,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x2A,0x47,0x00,0x00,0x53,0x65,0x72,0x65,0x6E,0x69,0x74,0x79,0x00,0x4E,0x48,0x20,0x55,0x4E,0x49,0x54,0x45,0x44,0x00,0x65,0x6C,0x6C,0x00,0x00,0x00,0x69,0x46,0x65,0x65,0x64,0x00,0x2D,0x2D,0x00,0x61,0x00,0x2E,0x00,0x00,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x43,0x61,0x6C,0x6D,0x20,0x42,0x65,0x66,0x6F,0x72,0x65,0x20,0x74,0x68,0x65,0x20,0x53,0x74,0x6F,0x72,0x6D,0x00,0x20,0x76,0x69,0x65,0x74,0x20,0x6E,0x61,0x6D,0x20,0x70,0x76,0x70,0x20,0x76,0x61,0x6F,0x20,0x64,0x61,0x79,0x00,0x70,0x65,0x6F,0x70,0x6C,0x65,0x00,0x72,0x20,0x6E,0x6F,0x20,0x47,0x52,0x42,0x2E,0x00,0x6F,0x00,0x00,0x00,0x0E,0x6C,0xBF,0x00,0x00,0x2D,0x47,0x00,0x00,0x2D,0x46,0x65,0x61,0x72,0x6C,0x65,0x73,0x73,0x20,0x6F,0x66,0x20,0x48,0x75,0x6E,0x67,0x61,0x72,0x79,0x2D,0x00,0x73,0x00,0x00,0x2D,0x44,0x61,0x72,0x69,0x75,0x73,0x2E,0x48,0x55,0x4E,0x2D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0x7A,0x65,0x72,0x65,0x74,0x65,0x74,0x65,0x6C,0x20,0x76,0xE1,0x72,0x6F,0x6D,0x20,0x61,0x20,0x4D,0x61,0x67,0x79,0x61,0x72,0x6F,0x6B,0x61,0x74,0x21,0x20,0x28,0x41,0x4B,0x54,0x49,0x56,0x20,0x47,0x55,0x49,0x4C,0x44,0x29,0x00,0x6E,0x20,0x74,0x72,0x61,0x64,0x65,0x00,0x65,0x6C,0x70,0x20,0x3A,0x44,0x00,0x5E,0x5E,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x32,0x47,0x00,0x00,0x52,0x6F,0x6F,0x73,0x74,0x65,0x72,0x73,0x20,0x42,0x72,0x6F,0x6F,0x64,0x00,0x67,0x65,0x72,0x73,0x00,0x65,0x61,0x72,0x73,0x00,0x52,0x6F,0x6F,0x73,0x74,0x65,0x72,0x48,0x65,0x61,0x6C,0x65,0x72,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57,0x65,0x20,0x6C,0x69,0x6B,0x65,0x61,0x20,0x74,0x68,0x65,0x20,0x72,0x6F,0x6F,0x73,0x74,0x65,0x72,0x00,0x20,0x67,0x75,0x69,0x6C,0x64,0x00,0x61,0x64,0x65,0x72,0x20,0x74,0x6F,0x20,0x63,0x68,0x65,0x63,0x6B,0x20,0x6C,0x69,0x73,0x74,0x00,0x20,0x35,0x2E,0x20,0x68,0x61,0x76,0x65,0x20,0x66,0x75,0x6E,0x00,0x74,0x00,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x34,0x47,0x00,0x00,0x46,0x6F,0x72,0x54,0x68,0x65,0x47,0x72,0x65,0x61,0x74,0x65,0x72,0x47,0x6F,0x6F,0x64,0x00,0x73,0x00,0x75,0x62,0x00,0x6F,0x00,0x42,0x6F,0x6E,0x64,0x2D,0x00,0x65,0x73,0x2D,0x5F,0x2D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x45,0x6E,0x6A,0x6F,0x79,0x00,0x6E,0x65,0x20,0x47,0x75,0x69,0x6C,0x64,0x20,0x46,0x61,0x6C,0x6C,0x73,0x2C,0x20,0x41,0x6E,0x6F,0x74,0x68,0x65,0x72,0x20,0x4D,0x75,0x73,0x74,0x20,0x52,0x69,0x73,0x65,0x00,0x46,0x61,0x6D,0x69,0x6C,0x79,0x20,0x2E,0x2E,0x2E,0x00,0x72,0x73,0x20,0x69,0x6E,0x20,0x67,0x75,0x69,0x6C,0x64,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x35,0x47,0x00,0x00,0x54,0x68,0x65,0x2D,0x4C,0x65,0x61,0x67,0x65,0x6E,0x64,0x61,0x72,0x79,0x00,0x69,0x47,0x48,0x54,0x53,0x2D,0x2D,0x54,0x52,0x00,0x4D,0x53,0x2E,0x41,0x4E,0x47,0x45,0x4C,0x2E,0x2E,0x2E,0x2E,0x2E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x68,0x69,0x00,0x57,0x65,0x6C,0x63,0x6F,0x6D,0x65,0x20,0x2A,0x2A,0x00,0x6F,0x75,0x6C,0x64,0x20,0x61,0x70,0x70,0x6C,0x79,0x20,0x74,0x6F,0x20,0x6F,0x75,0x72,0x20,0x6E,0x65,0x77,0x20,0x67,0x75,0x69,0x6C,0x64,0x20,0x58,0x41,0x4E,0x54,0x48,0x20,0x61,0x73,0x61,0x70,0x20,0x70,0x6C,0x65,0x61,0x73,0x65,0x00,0x73,0x00,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00};
    bldr.write_bytes(payload7, sizeof(payload7));
    character->write(bldr.to_packet());
}

/**
 * Sends the extra stats for a character. These are calculated from active buffs, and equipment.
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_extra_stats(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::ADDITIONAL_CHARACTER_STATS);
    bldr.write_int(1); // STR OJ
    bldr.write_int(2); // DEX OJ
    bldr.write_int(3); // REC OJ
    bldr.write_int(4); // INT OJ
    bldr.write_int(5); // WIS OJ
    bldr.write_int(6); // LUC OJ
    bldr.write_int(7); // Min atk
    bldr.write_int(8); // Max atk
    bldr.write_int(9); // Min magic atk
    bldr.write_int(10); // Max magic atk
    bldr.write_int(11); // Defense
    bldr.write_int(12); // Resistance
    character->write(bldr.to_packet());
}

/**
 * Sends the skill bars of the character
 *
 * @param character
 *      The character instance
 */
void PacketRepository::send_skillbars(Genesis::Game::Model::Entity::Player::Character* character) {
	Genesis::Common::Networking::Packets::PacketBuilder bldr(Genesis::Common::Packets::Opcodes::CHARACTER_SKILL_BARS);
    unsigned char payload22[] = {
        0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0xF1,0x02,0x00,0x00,0x00,0x00,0x00,0x01,0x64,0xF5,0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x64,0x68,0x02,0x00,0x00,0x00,0x00,0x01,0x09,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x64,0x1B,0x03,0x00,0x00,0x00,0x00,0x0A,0x01,0x64,0x1B,0x03,0x00,0x00,0x00,0x00,0x0A,0x02,0x64,0x1B,0x03,0x00,0x00,0x00,0x00,0x0B,0x00,0x64,0xE8,0x02,0x00,0x00,0x00,0x00,0x0B,0x01,0x64,0xE8,0x02,0x00,0x00,0x00,0x00,0x0B,0x02,0x64,0xE8,0x02};
    bldr.write_bytes(payload22, sizeof(payload22));
    character->write(bldr.to_packet());
}