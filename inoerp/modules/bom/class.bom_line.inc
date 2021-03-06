<?php

 class bom_line extends dbObject {

  public static $table_name = "bom_line";
  public static $primary_column = "bom_line_id";
  public static $parent_primary_column = "bom_header_id";
  public static $key_column = 'component_item_id_m';
  public static $module = "bom";
  public $field_a = [
   'bom_header_id',
   'bom_line_id',
   'bom_sequence',
   'routing_sequence',
   'component_item_id_m',
   'usage_basis',
   'usage_quantity',
   'auto_request_material_cb',
   'effective_start_date',
   'effective_end_date',
   'eco_number',
   'eco_implemented_cb',
   'planning_percentage',
   'yield',
   'include_in_cost_rollup_cb',
   'wip_supply_type',
   'supply_sub_inventory',
   'supply_locator',
   "ef_id",
   'created_by',
   'creation_date',
   'last_update_by',
   'last_update_date'
  ];
  //variables used for showing data
  public $fields_inForm_notInDataBase = [
   "component_item_number",
   "component_description",
   "component_uom",
   'bom_commonbom_line_id'
  ];
  public $fields_inHeader_needsToBeInserted_inPOST = [
   "bom_header_id"
  ];
  public $requiredField = [
   'bom_sequence',
   'routing_sequence',
   'component_item_id_m',
   'usage_quantity'
  ];
  public $bom_line_id;
  public $bom_header_id;
  public $bom_sequence;
  public $routing_sequence;
  public $component_item_id_m;
  public $usage_basis;
  public $usage_quantity;
  public $auto_request_material_cb;
  public $effective_start_date;
  public $effective_end_date;
  public $eco_number;
  public $eco_implemented_cb;
  public $planning_percentage;
  public $yield;
  public $include_in_cost_rollup_cb;
  public $wip_supply_type;
  public $supply_sub_inventory;
  public $supply_locator;
  public $created_by;
  public $creation_date;
  public $last_update_by;
  public $last_update_date;
  public $time;
  public $msg;
  public $component_item_number;
  public $component_description;
  public $component_uom;
  public $bom_commonbom_line_id;
  public $effecitve_items_only = false;
  public $revsion_effecitve_date = null;
  public $bom_header_revision_number = '';

  public function _before_save() {
   if (!empty($this->bom_header_id)) {
    $bom_header = bom_header::find_by_id($this->bom_header_id);
    if (!empty($bom_header->common_bom_item_id_m)) {
     echo "<br>Common BOM, Only Unique attributes are saved";
     $cbl = new bom_commonbom_line();

     foreach ($cbl->field_a as $k => $v) {
      if (!empty($this->$v)) {
       $cbl->$v = $this->$v;
      }
     }
     $cbl_i = $cbl->findBy_bomHeaderId_lineId();
     if ($cbl_i) {
      $cbl->bom_commonbom_line_id = $cbl_i->bom_commonbom_line_id;
     }
     $cbl->audit_trial();
     $cbl->save();
     return -90;
    }
   }
  }

  public static function find_all_by_bomHeaderId($bom_headerId) {
   $sql = " Select * from ";
   $sql .= self::$table_name;
   $sql .= " WHERE bom_header_id = '{$bom_headerId}'";
   $all_bom_lines = self::find_by_sql($sql);
   return $all_bom_lines;
  }

  public static function find_by_bomHeaderId_revNumber($bom_headerId, $rev_num = '') {
   if (empty($rev_num)) {
    $all_bom_lines = self::find_by_parent_id($bom_headerId);
   } else {
    $sql = " SELECT bl.bom_line_id, 	bl.bom_header_id, 	bl.bom_sequence, 	bl.routing_sequence, 	
     bl.component_item_id_m, 	bl.usage_basis, 	bl.usage_quantity, 	bl.auto_request_material_cb, 	
     bl.effective_start_date, 	bl.effective_end_date, 	bl.eco_number, 	bl.eco_implemented_cb, 	
     bl.planning_percentage, 	bl.yield, 	bl.include_in_cost_rollup_cb, 	bl.wip_supply_type, 	
     bl.supply_sub_inventory, 	bl.supply_locator, 	bl.ef_id, 	bl.created_by, 	bl.creation_date, 	
     bl.last_update_by, 	bl.last_update_date

     FROM      bom_line bl, 
     bom_header bh,
     inv_item_revision iir
     WHERE bh.bom_header_id = bl.bom_header_id 
     AND (bl.effective_end_date IS NULL OR bl.effective_end_date > iir.effective_start_date ) 
     AND bh.bom_header_id = '{$bom_headerId}' 
     AND iir.revision_name = '{$rev_num}'
     AND bh.item_id_m = iir.item_id_m
     AND bh.org_id = iir.org_id
     AND ( bl.effective_start_date <= iir.effective_end_date OR iir.effective_end_date IS NULL )
    ORDER BY bl.bom_sequence ASC";
    $all_bom_lines = self::find_by_sql($sql);
   }
//   echo $sql; pa($all_bom_lines);

   return !empty($all_bom_lines) ? $all_bom_lines : false;
  }

  public function findBy_parentId($id, $order_by_field = '', $order_by_seq = 'ASC') {
   if ($this->effecitve_items_only) {
    /*
     * Check if revision exists. If yes, use revision else find all items where 
     * (bl.effective_end_date IS NULL OR bl.effective_end_date > CURRENT_DATE
     */
    $boh_i = bom_header::find_by_id($id);
    $item_rev = inv_item_revision::find_currentRev_by_itemIdM_orgId($boh_i->item_id_m, $boh_i->org_id, $this->revsion_effecitve_date);
    if ($item_rev) {
     $all_bom_lines = self::find_by_bomHeaderId_revNumber($id, $item_rev->revision_name);
    } else {
     $sql = " SELECT bl.bom_line_id, 	bl.bom_header_id, 	bl.bom_sequence, 	bl.routing_sequence, 	
     bl.component_item_id_m, 	bl.usage_basis, 	bl.usage_quantity, 	bl.auto_request_material_cb, 	
     bl.effective_start_date, 	bl.effective_end_date, 	bl.eco_number, 	bl.eco_implemented_cb, 	
     bl.planning_percentage, 	bl.yield, 	bl.include_in_cost_rollup_cb, 	bl.wip_supply_type, 	
     bl.supply_sub_inventory, 	bl.supply_locator, 	bl.ef_id, 	bl.created_by, 	bl.creation_date, 	
     bl.last_update_by, 	bl.last_update_date

     FROM      bom_line bl, 
     bom_header bh
     
     WHERE bh.bom_header_id = bl.bom_header_id 
     AND (bl.effective_end_date IS NULL OR bl.effective_end_date > CURRENT_DATE ) 
     AND bh.bom_header_id = '{$id}' 
     ORDER BY bl.bom_sequence ASC   ";
     $all_bom_lines = self::find_by_sql($sql);
    }
    return !empty($all_bom_lines) ? $all_bom_lines : false;
   } else {
    return parent::findBy_parentId($id, $order_by_field, $order_by_seq);
   }
  }

 }

//end of inv_transaction class
?>