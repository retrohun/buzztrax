/*
 *    strmap version 2.0.1
 *
 *    ANSI C hash table for strings.
 *
 *	  Version history:
 *	  1.0.0 - initial release
 *	  2.0.0 - changed function prefix from strmap to sm to ensure
 *	      ANSI C compatibility 
 *	  2.0.1 - improved documentation 
 *
 *    strmap.c
 *
 *    Copyright (c) 2009, 2011, 2013 Per Ola Kristensson.
 *
 *    Per Ola Kristensson <pok21@cam.ac.uk> 
 *    Inference Group, Department of Physics
 *    University of Cambridge
 *    Cavendish Laboratory
 *    JJ Thomson Avenue
 *    CB3 0HE Cambridge
 *    United Kingdom
 *
 *    strmap is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    strmap is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with strmap.  If not, see <http://www.gnu.org/licenses/>.
 */  
#include "strpool.h"



{
  
   
 

{
  
   
 

{
  
   
 



{
  
  
  
    
  
  
  
  
    
    
  
  
  



sp_delete (StrPool * pool) 
{
  
  
  
  
    
  
  
  
  
  
    
    
    
    
      
      
      
      
    
    
    
    
  
  
  



sp_exists (const StrPool * pool, const char *key) 
{
  
  
  
  
    
  
  
    
  
  
  
  
  
    
  
  



sp_get (const StrPool * pool, const char *key) 
{
  
  
  
  
    
  
  
    
  
  
  
  
  
    
  
  



sp_put (StrPool * pool, const char *key, const char *value) 
{
  
  
  
  
  
  
    
  
  
    
  
  
  
  
      /* Get a pointer to the bucket the key string hashes to */ 
      index = hash (key) % pool->count;
  
  
      /* Check if we can handle insertion by simply replacing
       * an existing value in a key-value pair in the bucket.
       */ 
      if ((pair = get_pair (bucket, key)) != NULL) {
    
        /* The bucket contains a pair that matches the provided key,
         * change the value for that pair to the new value.
         */ 
        if (strlen (pair->value) < value_len) {
      
          /* If the new value is larger than the old value, re-allocate
           * space for the new larger value.
           */ 
          tmp_value = realloc (pair->value, (value_len + 1) * sizeof (char));
      
        
      
      
    
    
        /* Copy the new value into the pair that matches the key */ 
        strcpy (pair->value, value);
    
  
  
      /* Allocate space for a new key and value */ 
      new_key = malloc ((key_len + 1) * sizeof (char));
  
    
  
  
  
    
    
  
  
      /* Create a key-value pair */ 
      if (bucket->count == 0) {
    
        /* The bucket is empty, lazily allocate space for a single
         * key-value pair.
         */ 
        bucket->pairs = malloc (sizeof (Pair));
    
      
      
      
    
    
  
  
  else {
    
        /* The bucket wasn't empty but no pair existed that matches the provided
         * key, so create a new key-value pair.
         */ 
        tmp_pairs =
        realloc (bucket->pairs, (bucket->count + 1) * sizeof (Pair));
    
      
      
      
    
    
    
  
  
      /* Get the last pair in the chain for the bucket */ 
      pair = &(bucket->pairs[bucket->count - 1]);
  
  
  
      /* Copy the key and its value into the key-value pair */ 
      strcpy (pair->key, key);
  
  



sp_intern (StrPool * pool, const char *key) 
{
  
  
    
  
  



sp_get_count (const StrPool * pool) 
{
  
  
  
  
  
    
  
  
  
  
  
  
    
    
    
    
      
      
      
    
    
    
  
  



sp_enum (const StrPool * pool, sp_enum_func enum_func, const void *obj) 
{
  
  
  
  
    
  
  
    
  
  
  
  
  
    
    
    
    
      
      
      
    
    
    
  
  



/*
 * Returns a pair from the bucket that matches the provided key,
 * or null if no such pair exist.
 */ 
static Pair *
get_pair (Bucket * bucket, const char *key) 
{
  
  
  
  
    
  
  
  
  
    
      
        
      
    
    
    
  
  



/*
 * Returns a hash code for the provided string.
 */ 
    static unsigned long
hash (const char *str) 
{
  
  
  
    
  
  


