from pyradioconfig.parts.ocelot.profiles.Profile_LongRange import Profile_Long_Range_Ocelot
from pyradioconfig.parts.sol.profiles.sw_profile_outputs_common import sw_profile_outputs_common_sol

class Profile_Long_Range_Sol(Profile_Long_Range_Ocelot):

    def __init__(self):
        super().__init__()
        self._family = 'sol'
        self._sw_profile_outputs_common = sw_profile_outputs_common_sol()

    def _fixed_longrange_vars(self, model):
        super()._fixed_longrange_vars(model)
        model.vars.ofdm_option.value_forced = model.vars.ofdm_option.var_enum.OPT1