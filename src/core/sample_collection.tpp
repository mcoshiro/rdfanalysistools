//this gets included directly into histogram_collection.hxx in order to get general templates

/**
 * method to define data frame columns, see RInterface::Define
 * flags argument can be used to only define colums for certain samples
 */
template<typename F>
void SampleCollection::define(const char* name, F expression, std::vector<std::string> columns, std::set<int> flags) {
	for (unsigned int sample_idx = 0; sample_idx < samples.size(); sample_idx++) {
		if (flags.size() == 0 || flags.count(sample_flags[sample_idx])>0) {
			samples[sample_idx].data_frame() = samples[sample_idx].data_frame().Define(name, expression, columns);
		}
	}
}
