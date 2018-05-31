/*
 * benchmark.cc
 *
 * Copyright (C) 2018 OpenCog Foundation
 * All Rights Reserved
 *
 * Created by Vitaly Bogdanov <vsbogd@gmail.com> May 31, 2018
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <benchmark/benchmark.h>

#include <stdlib.h>

#include <opencog/atomspace/AtomSpace.h>
#include <opencog/query/BindLinkAPI.h>

using namespace opencog;

#define N atomspace.add_node
#define L atomspace.add_link

static std::string random_object() {
	static std::string objects[10] = { "apple", "cup", "hat", "face", "note",
	        "book", "door", "wall", "lamp", "arm" };
	return objects[rand() % 10];
}

using SetPositionFunction = std::function<void(Handle& bounding_box,
		const std::string& key, int position)>;

static void generate_frames(AtomSpace& atomspace,
		int bounding_box_count,
		const SetPositionFunction& set_position)
{
	const int BB_PER_FRAME = 16;
	int frame_count = bounding_box_count / BB_PER_FRAME;

	for (int frame_index = 0; frame_index < frame_count; frame_index++)
	{
		std::string frame_name = "Frame-" + std::to_string(frame_index);

		for (int bb_index = 0; bb_index < BB_PER_FRAME; bb_index++)
		{
			std::string bounding_box_name = "BB-" + std::to_string(frame_index) + "-"
			        + std::to_string(bb_index);
			Handle bounding_box = N(NODE, bounding_box_name);

			L(MEMBER_LINK, bounding_box, N(NODE, frame_name));
			L(INHERITANCE_LINK, bounding_box, N(CONCEPT_NODE, random_object()));

			set_position(bounding_box, "Left", rand() % 1000);
			set_position(bounding_box, "Top", rand() % 1000);
			set_position(bounding_box, "Right", rand() % 1000);
			set_position(bounding_box, "Bottom", rand() % 1000);
		}
	}
}

static void BM_UpperThanBoundingBoxesNodes(benchmark::State& state)
{
	AtomSpace atomspace;

	const auto& set_position = [&atomspace](Handle& bounding_box,
			const std::string& key, int position)
	{
		L(MEMBER_LINK,
		        L(INHERITANCE_LINK,
		                N(NUMBER_NODE, std::to_string(position)),
		                N(CONCEPT_NODE, key)),
		        bounding_box);
	};

	generate_frames(atomspace, state.range(0), set_position);

	Handle bind_link = L(BIND_LINK,
	        L(VARIABLE_LIST,
	                N(VARIABLE_NODE, "$UpperBB"),
	                N(VARIABLE_NODE, "$LowerBB"),
	                N(VARIABLE_NODE, "$Frame"),
	                N(VARIABLE_NODE, "$Top"),
	                N(VARIABLE_NODE, "$Bottom")),
	        L(AND_LINK,
	                L(MEMBER_LINK,
	                        N(VARIABLE_NODE, "$UpperBB"),
	                        N(VARIABLE_NODE, "$Frame")),
	                L(MEMBER_LINK,
	                        N(VARIABLE_NODE, "$LowerBB"),
	                        N(VARIABLE_NODE, "$Frame")),
	                L(MEMBER_LINK,
	                        L(INHERITANCE_LINK,
	                                N(VARIABLE_NODE, "$Top"),
	                                N(CONCEPT_NODE, "Top")),
	                        N(VARIABLE_NODE, "$LowerBB")),
	                L(MEMBER_LINK,
	                        L(INHERITANCE_LINK,
	                                N(VARIABLE_NODE, "$Bottom"),
	                                N(CONCEPT_NODE, "Bottom")),
	                        N(VARIABLE_NODE, "$UpperBB")),
	                L(GREATER_THAN_LINK,
	                        N(VARIABLE_NODE, "$Top"),
	                        N(VARIABLE_NODE, "$Bottom"))),
	        L(EVALUATION_LINK,
	                N(PREDICATE_NODE, "Higher-Than"),
	                L(LIST_LINK,
	                        N(VARIABLE_NODE, "$UpperBB"),
	                        N(VARIABLE_NODE, "$LowerBB"))));

	for (auto _ : state)
	{
		bindlink(&atomspace, bind_link);
	}
}
BENCHMARK(BM_UpperThanBoundingBoxesNodes)->Unit(benchmark::kMillisecond)
		->Arg(32768)->UseRealTime();

static void BM_UpperThanBoundingBoxesValues(benchmark::State& state)
{
	AtomSpace atomspace;

	const auto& set_position = [&atomspace](Handle& bounding_box,
			const std::string& key, int position)
	{
		bounding_box->setValue(N(PREDICATE_NODE, key),
				createFloatValue((double) position));
	};

	generate_frames(atomspace, state.range(0), set_position);

	Handle bind_link = L(BIND_LINK,
	        L(VARIABLE_LIST,
	                N(VARIABLE_NODE, "$UpperBB"),
	                N(VARIABLE_NODE, "$LowerBB"),
	                N(VARIABLE_NODE, "$Frame")),
	        L(AND_LINK,
	                L(MEMBER_LINK,
	                        N(VARIABLE_NODE, "$UpperBB"),
	                        N(VARIABLE_NODE, "$Frame")),
	                L(MEMBER_LINK,
	                        N(VARIABLE_NODE, "$LowerBB"),
	                        N(VARIABLE_NODE, "$Frame")),
	                L(GREATER_THAN_LINK,
	                        L(VALUE_OF_LINK,
	                                N(VARIABLE_NODE, "$LowerBB"),
	                                N(PREDICATE_NODE, "Top")),
	                        L(VALUE_OF_LINK,
	                                N(VARIABLE_NODE, "$UpperBB"),
	                                N(PREDICATE_NODE, "Bottom")))),
	        L(EVALUATION_LINK,
	                N(PREDICATE_NODE, "Higher-Than"),
	                L(LIST_LINK,
	                        N(VARIABLE_NODE, "$UpperBB"),
	                        N(VARIABLE_NODE, "$LowerBB"))));

	for (auto _ : state)
	{
		bindlink(&atomspace, bind_link);
	}
}
BENCHMARK(BM_UpperThanBoundingBoxesValues)->Unit(benchmark::kMillisecond)
		->Arg(32768)->UseRealTime();

int main(int argc, char** argv)
{
	logger().set_level(Logger::INFO);
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
}
