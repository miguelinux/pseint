#include "EntityVerifier.h"

#define check(x) { if (!(x)) asm("int3"); }

void VerifyLinks(Entity *e, bool recursive) {
	if (e->prev) {
		check( e->prev->next == e );
		check( e->parent == e->prev->parent );
		check( e->child_id == -1 );
	} else {
		if (e->child_id!=-1) {
			check( e->parent->n_child > e->child_id );
			check( e->parent->child[e->child_id] == e );
		}
	}
	if (e->next) {
		check( e->next->prev == e );
		check( e->parent == e->next->parent );
	}
	check( (e->child==NULL) == (e->n_child==0) );
	for(int i=0;i<e->n_child;i++) {
		if (e->child[i]) {
			check( e->child[i]->child_id == i );
			check( e->child[i]->parent == e );
		}
	}
	if (recursive) {
		if (e->next) VerifyLinks(e->next,true);
		for(int i=0;i<e->n_child;i++) 
			if (e->child[i]) VerifyLinks(e->child[i],true);
	}
}

void VerifyAll( ) {
	Entity *e0 = Entity::all_any, *e1 = Entity::all_any;
	do {
		VerifyLinks(e0,false);
		e0 = e0->all_next;
	} while(e0!=e1);
}

