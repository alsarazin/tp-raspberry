#include "sched.h"
#include "phyAlloc.h"


void init_ctx(struct ctx_s* ctx, func_t f, unsigned int stack_size){
        ctx->stackPointer = (unsigned int)phyAlloc_alloc(stack_size) + stack_size - 14*4;
        ctx->linkR = (unsigned int) start_current_process;
}

/*void __attribute__ ((naked)) switch_to(struct ctx_s* ctx){

	//1. Sauvegarder le contexte courant
	__asm("push {r0-r12}");	

	__asm("mov %0, sp" : "=r"(current_ctx->stackPointer));	
	__asm("mov %0, lr" : "=r"(current_ctx->linkR));

	//2. Changer de contexte courant 
	//(faire pointer current_ctx vers le contexte ctx passé en paramètre)
	current_ctx = ctx;

	//3. Restaurer ce nouveau contexte

	__asm("mov sp, %0" : : "r"(current_ctx->stackPointer));	
	__asm("mov lr, %0" : : "r"(current_ctx->linkR));
	
	__asm("pop {r0-r12}");

	//4. Sauter à l’adresse de retour du contexte restauré
	__asm("bx lr");

}*/

void init_pcb(pcb_s* aPCB, func_t f, void* args, ctx_s* ctx, unsigned int stackSize){
	
	aPCB->state=NEW;
	aPCB->function = f;
	aPCB->functionArgs=args;
	aPCB->ctx=ctx;
	aPCB->stack_size = stackSize;
}

void create_process(func_t f, void* args, unsigned int stack_size){
	
	ctx_s* ctx;
	init_ctx(ctx, f, stack_size);

	pcb_s* pcb = phyAlloc_alloc(sizeof(pcb_s));

	init_pcb(pcb, f, args, ctx, stack_size);

	if (first == NULL){
		first = pcb;
		last = pcb;
	}
	else{
		last->next = pcb;
		last = pcb;
		last->next = first;
	}
}

void start_current_process(){
	current_process->state = RUNNING;
	current_process->function(current_process->functionArgs);
}

void elect(){
	current_process=current_process->next;
}

void start_sched(){
	current_process=first;
}

void __attribute__ ((naked)) ctx_switch(){


	//1. sauvegarde le contexte du processus en cours d’exécution
	__asm("push {r0-r12}");	

	__asm("mov %0, sp" : "=r"(current_process->ctx->stackPointer));	
	__asm("mov %0, lr" : "=r"(current_process->ctx->linkR));

	//2. demande au scheduler d’élire un nouveau processus
	elect();

	//3. restaure le contexte du processus élu
	__asm("mov sp, %0" : : "r"(current_process->ctx->stackPointer));	
	__asm("mov lr, %0" : : "r"(current_process->ctx->linkR));
	
	__asm("pop {r0-r12}");
	
	__asm("bx lr");
	
}



