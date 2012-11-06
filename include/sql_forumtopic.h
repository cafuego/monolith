/* $Id$ */

#define FT_TABLE "forumtopic"

int add_to_topiclist(topiclist_t element, topiclist_t ** list);
int dest_topiclist(topiclist_t * list);
int mono_sql_ft_list_topics_by_forum(unsigned int forum_id, topiclist_t ** p);

/* eof */
