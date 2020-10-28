//this gets included directly into histogram_collection.hxx in order to get general templates

/**
 * method to define data frame columns, see RInterface::Define
 * flags argument can be used to only define colums for certain samples
 */
template<typename F>
SampleCollection* SampleCollection::define(const char* name, F expression, const std::vector<std::string> columns, std::vector<std::string> flags) {
  if (flags.size() > 0) {
    //if flags provided, define this column only for samples with flag flag
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      for (std::string flag : flags) {
        if (samples[sample_idx]->check_flag(flag)) {
      	samples[sample_idx]->data_frame() = samples[sample_idx]->data_frame().Define(name, expression, columns);
          break;
        }
      }
    }
  }
  else {
    //if no flags provided, define for all samples
    for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
      samples[sample_idx]->data_frame() = samples[sample_idx]->data_frame().Define(name, expression, columns);
    }
  }
  return this;
}

///**
// * method to define data frame columns, see RInterface::Define
// * flags argument can be used to only define colums for certain samples
// */
//template<typename F>
//void SampleCollection::define(ColumnDefinition<F> column_definition, std::vector<std::string> flags) {
//  for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
//    for (std::string flag : flags) {
//      if (samples[sample_idx]->check_flag(flag)) {
//    	samples[sample_idx]->data_frame() = samples[sample_idx]->data_frame().Define(
//          column_definition.name, *column_name.evaluate, column_name.arguments);
// 	break;
//      }
//    }
//  }
//}
