void do_skin(CHAR_DATA * ch, char *argument) {

    char arg[MAX_INPUT_LENGTH];

    OBJ_DATA *obj, *corpse;

    OBJ_DATA * startobj;

    bool knife;

    //gotta make sure they are in a position to be able to skin 
    switch (ch->substate) {

        default:

            if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {

                send_to_char("You can't concentrate enough for that.\n\r", ch);

                return;

            }

            if (ch->mount) {

                send_to_char("You can't do that while mounted.\n\r", ch);

                return;

            }

            if (IS_AFFECTED(ch, AFF_FLYING)) {

                send_to_char("You are flying high above the ground.\n\r", ch);

                return;

            }

            one_argument(argument, arg);

            if (arg[0] == '\0') {

                send_to_char("What are you trying to skin?\n\r", ch);

                return;

            }


            if ((corpse = get_obj_here(ch, argument)) == NULL) {

                send_to_char("You cannot find that here.\n\r", ch);

                return;

            }

            if (corpse->item_type != ITEM_CORPSE_NPC) {

                send_to_char("You can only skin the corpses of creatures.\n\r", ch);

                return;

            }

            if (corpse->skinamount < 1) {

                send_to_char("That corpse seems to have been skinned already.\r\n", ch);

                return;

            } else

                //roundtime        
                add_timer(ch, TIMER_DO_FUN, UMIN(skill_table[gsn_skin]->beats / 10, 3), do_skin, 1);

            ch->alloc_ptr = str_dup(arg);

            send_to_char("You begin skinning...\n\r", ch);

            act(AT_PLAIN, "$n begins skinning...", ch, NULL, NULL, TO_ROOM);

            return;

        case 1:

            if (!ch->alloc_ptr) {

                send_to_char("Your skinning was interrupted!\n\r", ch);

                act(AT_PLAIN, "$n's skinning was interrupted!", ch, NULL, NULL, TO_ROOM);

                bug("do_skin: alloc_ptr NULL", 0);

                return;

            }

            strcpy(arg, ch->alloc_ptr);

            DISPOSE(ch->alloc_ptr);

            break;

        case SUB_TIMER_DO_ABORT:

            DISPOSE(ch->alloc_ptr);

            ch->substate = SUB_NONE;

            send_to_char("You stop skinning...\n\r", ch);

            act(AT_PLAIN, "$n stops skinning...", ch, NULL, NULL, TO_ROOM);

            return;

    }

    ch->substate = SUB_NONE;

    /*
     * not having a knife makes it harder to succeed
     *  
     * I haz knife?
     */
    knife = FALSE;

    for (obj = ch->first_carrying; obj; obj = obj->next_content)

        if (obj->item_type == ITEM_CARVINGKNIFE) {

            knife = TRUE;

            break;

        }


            /*
             * 4 times harder to skin something without a knife 
             */
            if (can_use_skill(ch, (number_percent() * (knife ? 1 : 4)), gsn_skin)) {

                if (corpse->skinamount < 2) {
                    send_to_char("There isn't much hide left.\n\r", ch);
                    send_to_char("\n\r", ch);
                    send_to_char("\n\r", ch);
                }

                send_to_char("You skin the corpse!\n\r", ch);

                act(AT_PLAIN, "$n skins a corpse!", ch, NULL, NULL, TO_ROOM);

                learn_from_success(ch, gsn_skin);


                learn_noncombat(ch, SK_NATURE);
                // This is if the skin is successful 
                if (number_range(1, 300) < obj->skinamount + get_curr_per(ch) + ch->pcdata->noncombat[SK_NATURE] && obj->skinamount > 0) {

                    MATERIAL_DATA * material;

                    for (material = first_material; material; material = material->next) {
                        // This sector type is going to have to add a sector type to mob corpses so can skin for them
                        if (((!knife && number_percent() < 50) || (knife)) && (material->hide_type == obj->hide_type) && (number_percent() < material->rarity)) {

                            /*
                             * Can't get magical anything in mana dead areas 
                             */
                            if (xIS_SET(material->extra_flags, ITEM_MAGIC)
                                    && ch->in_room->area->weather->mana < -100)

                                continue;

                            obj = make_ore(material->number);

                            if (obj) {

                                obj_to_char(obj, ch);

                                corpse->skinamount--; //removing a skin from the corpse


                                break;

                            }

                        }

                    }

                }


                return;

            }

        // failed skill check mangle the skin    
        learn_from_failure(ch, gsn_skin);

        send_to_char("Your mangle the skin beyond repair...\n\r", ch);

        act(AT_PLAIN, "$n's mangles the skin beyond repair...", ch, NULL, NULL, TO_ROOM);
        // remove a layer of skin from the corpse 
        corpse->skinamount--;

        if (corpse->skinamount < 2)

            send_to_char("There isn't much hide left.\r\n", ch);

        return;

    }
