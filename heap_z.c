//----------------------------------------------------------------------------
// Работа с heap
// Структура heap:
// {first_heap_mcb| memory part} {heap_mcb| memory part}...{heap_mcb| memory part}
//  heap_mcb - описатель элемента памяти (Memory Comtrol Block)
//            memory part - элемент памяти, который описывается соответствующим MCB
//  Поле mcb.next описателя последнего MCB всегда указывает
// 	на первый MCB - циклическая структура.
//  Указатель mcb.prev первого MCB указывает сам на себя.
//----------------------------------------------------------------------------
//#include "stdafx.h"

#include <stdlib.h>
#include "printf_hal.h"


#ifdef FREERTOS
#include "FreeRTOS.h"
#define HEAP_ALIGN	portBYTE_ALIGNMENT
#else
#define HEAP_ALIGN	(8)
#endif

#include "heap_z.h"

#define USE_FULL_SCAN 	0 	// Полный перебор свободных блоков с целью
				// найти свободный блок совпадающий по размеру с
				// запрашиваемым.

#define BYTE unsigned char

// определяем массив под кучу.
#define HEAP_RAM_ARRAY_SIZE (1024*4-256)
static unsigned char heap_ram_array[ HEAP_RAM_ARRAY_SIZE ];

//---------------------------------------------------------------------------
heap_t system_heap;
//----------------------------------------------------------------------------
void heapadd( heap_t *heap, heap_mcb *addr, int size );
// Добавляет к 'heap' еще один отдельно находящийся блок.
// *heap - Указатель на основной уже проинициализированный heap
//----------------------------------------------------------------------------
void heapadd( heap_t *heap, heap_mcb *xptr, int size )
{
	heap_mcb *tptr = heap->freem;
    // Формирование нового MCB в блоке
	xptr->next = tptr;
    xptr->prev = tptr;
    xptr->ts.size = size-sizeof(heap_mcb);
    xptr->ts.type = MARK_FREE;
    xptr->owner = 0;
    // Reinit Primary MCB
    tptr->next = xptr;
	// xptr->prev = xptr; Баг а-ля copy-paste внесенный при подготовке к публикации
}

//---------------------------------------------------------------------------
// Инициализация Heap - обязательный вызов перед использованием
// malloc_z() и free_z()
//---------------------------------------------------------------------------
void init_system_heap(void)
{
	system_heap.start = (heap_mcb*)&heap_ram_array[0];
	system_heap.hsize = HEAP_RAM_ARRAY_SIZE;
	system_heap.freem = system_heap.start;

	heapinit( &system_heap );
//	heapadd( &system_heap, ...., .... );

}

//----------------------------------------------------------------------------
// Инициализирует 'heap'.
// *heap - Указатель на структуру описываюшую heap
//----------------------------------------------------------------------------
void heapinit( heap_t *heap )
{
	heap_mcb *fmcb;

    fmcb = heap->start;
    // Циклическая структура
    fmcb->next = fmcb;
    // Указатель на предыдущий MCB указывает сам на себя
    fmcb->prev = fmcb;
    // Размер области памяти
    fmcb->ts.size = heap->hsize - sizeof(heap_mcb);
    // Область памяти свободна
    fmcb->ts.type = MARK_FREE;
    fmcb->owner = 0;

// После инициализации heap представляет собой один свободный блок,
// который имеет размер heap минус размер MCB.

}

//----------------------------------------------------------------------------
// malloc()
//----------------------------------------------------------------------------
void *malloc_z( heap_t *heap, size_t size, int type, void *owner )
{
//	vTaskSuspendScheduler();

heap_mcb *tptr = heap->freem; // Поиск начинается с первого свободного
#if( USE_FULL_SCAN )
heap_mcb *xptr = NULL;
#else
heap_mcb *xptr;
#endif
void *fptr;
int free_cnt = 0;

#ifdef HEAP_ALIGN
    if( size &( HEAP_ALIGN - 1 ) )
    	size = size + ( HEAP_ALIGN - ( size &( HEAP_ALIGN - 1 )) );
#endif
	for( ; ; )
    {   if( tptr->ts.type == MARK_FREE )
        {
#if( USE_FULL_SCAN )
#else
			++free_cnt;
#endif
			if( tptr->ts.size == size )					// Требуемый и найденный размеры памяти равны?
            {	tptr->owner = owner;
            	tptr->ts.type = type;              		// Резервируем блок
                fptr = (BYTE *)tptr+sizeof(heap_mcb);
#if( USE_FULL_SCAN )
				++free_cnt;
#endif
				break;
			}
#if( USE_FULL_SCAN )
			else if( xptr == NULL )
			{   if( tptr->ts.size >= ( size + sizeof(heap_mcb) ) ) // Массив достаточен для размещения блока и его MCB?
					xptr = tptr;
			    ++free_cnt;
			}
#else
            else if( tptr->ts.size >= ( size + sizeof(heap_mcb) ) ) // Массив достаточен для размещения блока и его MCB?
            { 	// Create new free MCB in parent's MCB tail
				xptr = (heap_mcb *)( (BYTE *)tptr + sizeof(heap_mcb) + size );
                xptr->next = tptr->next;
                xptr->prev = tptr;
                xptr->ts.size = ( tptr->ts.size - size - sizeof(heap_mcb) );
                xptr->ts.type = MARK_FREE;
                // Reinit curent MCB
                tptr->next = xptr;
                tptr->ts.size = size;
                tptr->ts.type = type; 		// Mark block as used
                tptr->owner = owner;
                // Если следующий MCB не последний, то mcb.prev следующего за ним
                // должно теперь указывать на выделенный (xptr) MCB
                if( xptr->next != heap->start )
                	( xptr->next )->prev = xptr;
                fptr = (BYTE *)tptr + sizeof(heap_mcb); 	// Valid pointer
       			break;
			}
#endif
        }
        // Get ptr to next MCB
        tptr = tptr->next;
		if( tptr == heap->start )	// End of heap?
		{
#if( USE_FULL_SCAN )
			if( xptr != NULL )
			{	tptr = xptr;
				// Create new free MCB in parent's MCB tail
				xptr = (heap_mcb *)( (BYTE *)tptr + sizeof(heap_mcb) + size );
                xptr->next = tptr->next;
                xptr->prev = tptr;
                xptr->ts.size = ( tptr->ts.size - size - sizeof(heap_mcb) );
                xptr->ts.type = MARK_FREE;
                // Reinit curent MCB
                tptr->next = xptr;
                tptr->ts.size = size;
                tptr->ts.type = type; 		// Mark block as used
                tptr->owner = owner;
                // Если следующий MCB не последний, то mcb.prev следующего за ним
                // должно теперь указывать на выделенный (xptr) MCB
                if( xptr->next != heap->start )
                	( xptr->next )->prev = xptr;
                fptr = (BYTE *)tptr + sizeof(heap_mcb); 	// Valid pointer
       			break;
			}
			else
#endif
			{	fptr = NULL; 			// No Memory
            	break;
			}
        }
    }

 	if( ( free_cnt == 1 )&&( fptr ) )	// Был занят первый свободный блок памяти?
 		heap->freem = tptr->next; 		// Указатель 'первый свободный' на следующий MCB
										// он или свободен или по крайней мере ближе к следующему свободному
//    xTaskResumeScheduler();

    return( fptr );
}

//----------------------------------------------------------------------------
// free()
//----------------------------------------------------------------------------
void free_z( heap_t *heap, void *mem_ptr )
{
//	vTaskSuspendScheduler();
	heap_mcb *xptr;
	heap_mcb *tptr = (heap_mcb *)( (BYTE *)mem_ptr - sizeof(heap_mcb) );

	// В общем надо контролировать _все_ :( указатели на попадание в RAM, иначе будет exception :(
	// Или использовать тупой перебор MCB и сравнивать с mem_ptr
	// Пока? только mem_ptr и то по одной границе.
	// Перекрестная проверка для определения валидности
	xptr = tptr->prev;
    if ( ((xptr != tptr) && ( xptr->next != tptr )) || ( mem_ptr < (void*)heap->start ) )
    {
        //xTaskResumeScheduler();
        return;
    }
	// Valid pointer present ------------------------------------------------
    tptr->ts.type = MARK_FREE; 		// Mark as "free"
	// Check Next MCB
    xptr = tptr->next;
//  // Если следующий MCB свободен и не первый в heap..
//	if( ( xptr->ts.type == MARK_FREE )&&( xptr != heap->start ) )  	// Баг найденный Сергеем Борщем
    // Если следующий MCB свободен и примыкает к текущему
	
	if( ( xptr->ts.type == MARK_FREE )&&( (unsigned int)xptr == ( (unsigned int)tptr + sizeof(heap_mcb) + tptr->ts.size ) ) )
    {   // Объединяем текущий (tptr) и следующий (xptr) MCB
		tptr->ts.size = tptr->ts.size + xptr->ts.size + sizeof(heap_mcb);
		tptr->next = xptr->next;
		// Если следующий MCB не последний, то меняем в нем mcb.prev на текущий
		xptr = xptr->next;
        if( xptr != heap->start )
            xptr->prev = tptr;
	}
	// Check previous MCB
    xptr = tptr->prev;
//  // Если предыдущий MCB свободен и текущий не первый в heap...
//  if( ( xptr->ts.type == MARK_FREE )&&( tptr != heap->start ) ) 	// Аналогичный баг
    // Если предыдущий MCB свободен и примыкает к текущему
  	if( ( xptr->ts.type == MARK_FREE )&&( (unsigned int)tptr == ( (unsigned int)xptr + sizeof(heap_mcb) + xptr->ts.size ) ) )
    {	// Объединяем текущий (tptr) и предыдущий (xptr) MCB
 		xptr->ts.size = xptr->ts.size + tptr->ts.size + sizeof(heap_mcb);
   		xptr->next = tptr->next;
		// Если следующий MCB не последний, то меняем в нем mcb.prev на текущий
		tptr = tptr->next;
		if( tptr != heap->start )
        	tptr->prev = xptr;
		tptr = xptr;			// tptr всегда на освободившийся блок.
	}
	// Установка heap->freem для более быстрого перебора
 	if( tptr < heap->freem ) 	// Осводившийся блок находится перед считающимся первым свободным?
 		heap->freem = tptr; 	// Новый указатель на первый 'free'

//	xTaskResumeScheduler();
}
