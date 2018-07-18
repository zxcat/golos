#pragma once

namespace golos { namespace plugins { namespace social_network {
    using namespace golos::chain;
    
    #ifndef SOCIAL_NETWORK_SPACE_ID
    #define SOCIAL_NETWORK_SPACE_ID 10
    #endif

        enum social_network_types {
            comment_content_object_type = (SOCIAL_NETWORK_SPACE_ID << 8)
        };


        class comment_content_object
                : public object<comment_content_object_type, comment_content_object> {
        public:
            comment_content_object() = delete;

            template<typename Constructor, typename Allocator>
            comment_content_object(Constructor&& c, allocator <Allocator> a)
                    :title(a), body(a), json_metadata(a) {
                c(*this);
            }

            id_type id;

            comment_id_type   comment;

            shared_string title;
            shared_string body;
            shared_string json_metadata;

            uint32_t block_number;
        };


        using comment_content_id_type = object_id<comment_content_object>;

        struct by_comment;
        struct by_block_number;

        typedef multi_index_container<
              comment_content_object,
              indexed_by<
                 ordered_unique<tag<by_id>, member<comment_content_object, comment_content_id_type, &comment_content_object::id>>,
                 ordered_unique<tag<by_comment>, member<comment_content_object, comment_id_type, &comment_content_object::comment>>,
                 ordered_non_unique<tag<by_block_number>, member<comment_content_object, uint32_t, &comment_content_object::block_number>>>,
            allocator<comment_content_object>
        > comment_content_index;
} } }


CHAINBASE_SET_INDEX_TYPE(
    golos::plugins::social_network::comment_content_object, 
    golos::plugins::social_network::comment_content_index
)